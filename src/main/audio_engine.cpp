#include "audio_engine.h"

#include "audio_graph.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <vector>

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
    ma_device_config config = ma_device_config_init(ma_device_type_playback);
    config.playback.format = ma_format_f32;  // Set to ma_format_unknown to use the device's native format.
    config.playback.channels = 2;            // Set to 0 to use the device's native channel count.
    config.sampleRate = 48000;               // Set to 0 to use the device's native sample rate.
    config.dataCallback = s_dataCallback;    // This function will be called when miniaudio needs more data.
    config.pUserData = this;                 // Can be accessed from the device object (device.pUserData).
    config.noPreSilencedOutputBuffer = true;
    config.noClip = true;

    if (ma_device_init(NULL, &config, &m_device) != MA_SUCCESS) {
        return -1;  // Failed to initialize the device.
    }

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
    float* out = (float*)pOutput;
    float sum2 = 0.0f;
    for (ma_uint32 i = 0; i < frameCount; ++i) {
        float sample = m_node_graph ? m_node_graph->outputNode()->generate(0) : 0.0;
        *out++ = sample;
        *out++ = sample;
        sum2 += sample * sample;
        m_history[m_p_hist++] = sample;
        if (m_p_hist == HISTORY_SIZE) {
            m_p_hist = 0;
        }
    }
    float rms = sqrt(sum2 / frameCount);
    m_db = 20 * log10(rms);
}
