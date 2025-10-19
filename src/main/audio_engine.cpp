#include "audio_engine.h"

#include "audio_graph.h"

#include <assert.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <print>
#include <vector>
#define NOMINMAX
#include <miniaudio.h>

class AudioEngineImpl : public AudioEngine {
   public:
    AudioEngineImpl();
    ~AudioEngineImpl();
    int init() override;
    void setGraph(AuNodeGraphPtr graph) override;
    AuNodeGraphPtr getGraph() override;
    float getDb() const override;
    const std::vector<float>& getHistory() const override;
    size_t getHistoryPos() const override;

    static const size_t HISTORY_SIZE = 10 * 48000;
   private:
    static void s_dataCallback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount);
    void dataCallback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount);
    ma_context m_context;
    ma_device m_device;
    AuNodeGraphPtr m_node_graph;
    float m_db;
    std::vector<float> m_history;
    size_t m_p_hist;
};

std::unique_ptr<AudioEngine> AudioEngine::create() {
    return std::make_unique<AudioEngineImpl>();
}

AudioEngineImpl::AudioEngineImpl() {
    m_node_graph = 0;
    m_db = 0;
    m_device = {};
    m_history.resize(HISTORY_SIZE);
    m_p_hist = 0;
}

AudioEngineImpl::~AudioEngineImpl() {
    ma_device_uninit(&m_device);
}

int AudioEngineImpl::init() {
    if (ma_context_init(0, 0, 0, &m_context)) {
        std::print("Error: initalizing context\n");
        return -1;
    }
    ma_device_info* pPlaybackInfos;
    ma_uint32 playbackCount;
    ma_device_info* pCaptureInfos;
    ma_uint32 captureCount;
    if (ma_context_get_devices(&m_context, &pPlaybackInfos, &playbackCount, &pCaptureInfos, &captureCount) != MA_SUCCESS) {
        std::print("Error: enumerating devices\n");
        return -1;
    }
    for (int i = 0; i < playbackCount; ++i) {
        auto& info = pPlaybackInfos[i];
        std::print("Device: {}\n", info.name);
        for (int j = 0; j < info.nativeDataFormatCount; ++j) {
            auto& format = info.nativeDataFormats[j];
            std::print("  Format: {}, {} Hz, {} channels, flags: {}\n", (int)format.format, format.sampleRate, format.channels, format.flags);
        }
    }

    ma_device_config config = ma_device_config_init(ma_device_type_playback);
    config.playback.format = ma_format_unknown;  // Set to ma_format_unknown to use the device's native format.
    config.playback.channels = 0;            // Set to 0 to use the device's native channel count.
    config.playback.shareMode = ma_share_mode_exclusive;
    config.sampleRate = 0;               // Set to 0 to use the device's native sample rate.
    config.dataCallback = s_dataCallback;    // This function will be called when miniaudio needs more data.
    config.pUserData = this;                 // Can be accessed from the device object (device.pUserData).
    config.noPreSilencedOutputBuffer = true;
    config.noClip = true;
    config.noFixedSizedCallback = true;
    config.noDisableDenormals = true;
    config.periods = 0;
    config.periodSizeInFrames = 200;
    config.performanceProfile = ma_performance_profile_low_latency;
    config.wasapi.noAutoConvertSRC = true;
    config.wasapi.usage = ma_wasapi_usage_pro_audio;


    if (ma_device_init(NULL, &config, &m_device) != MA_SUCCESS) {
        return -1;  // Failed to initialize the device.
    }
    std::print("Using audio device: {}\n", m_device.playback.name);
    std::print("  Sample rate:      {} Hz\n", m_device.playback.internalSampleRate);
    std::print("  Format:           {}\n", (int)m_device.playback.internalFormat);
    std::print("  Frames:           {}\n", m_device.playback.internalPeriodSizeInFrames);
    std::print("  Periods:          {}\n", m_device.playback.internalPeriods);
    std::print("  Buffer length:    {} ms\n", 1000 * m_device.playback.internalPeriodSizeInFrames * m_device.playback.internalPeriods /
                                                  m_device.playback.internalSampleRate);


    ma_device_start(&m_device);  // The device is sleeping by default so you'll need to start it manually.
    return 0;
}

void AudioEngineImpl::setGraph(AuNodeGraphPtr node_graph) {
    m_node_graph = node_graph;
}

AuNodeGraphPtr AudioEngineImpl::getGraph() {
    return m_node_graph;
}

float AudioEngineImpl::getDb() const {
    return m_db;
}

const std::vector<float>& AudioEngineImpl::getHistory() const {
    return m_history;
}

size_t AudioEngineImpl::getHistoryPos() const {
    return m_p_hist;
}

void AudioEngineImpl::s_dataCallback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
    ((AudioEngineImpl*)pDevice->pUserData)->dataCallback(pDevice, pOutput, pInput, frameCount);
}

void AudioEngineImpl::dataCallback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
    // std::print("Frame count: {}\n", frameCount);
    int channels = 2;
    if (m_node_graph == 0) {
        int size = 0;
        switch (pDevice->playback.format) {
            case ma_format_f32:
                size = 4;
                break;
            case ma_format_s16:
                size = 2;
                break;
        }
        memset(pOutput, 0, size * frameCount * channels);
        return;
    }

    float* out_f = (float*)pOutput;
    short* out_s = (short*)pOutput;
    float sum2 = 0.0f;


    for (ma_uint32 i = 0; i < frameCount; ++i) {
        float sample = std::max(-1.0f, std::min(1.0f, m_node_graph->outputNode()->generate(0)));
        switch (pDevice->playback.format) {
            case ma_format_f32:
                *out_f++ = sample;
                *out_f++ = sample;
                break;
            case ma_format_s16:
                short s = (short)(sample * 16000);
                *out_s++ = s;
                *out_s++ = s;
                break;
        }
        sum2 += sample * sample;
        m_history[m_p_hist++] = sample;
        if (m_p_hist == HISTORY_SIZE) {
            m_p_hist = 0;
        }
    }
    float rms = sqrt(sum2 / frameCount);
    m_db = 20 * log10(rms);
}
