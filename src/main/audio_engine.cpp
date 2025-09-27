#include "audio_engine.h"

#include "audio_graph.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include <miniaudio.h>

class SineGenerator {
   public:
    SineGenerator() {
        m_freq = 440;
        m_phase = 0;
        m_multiplier = 2.0 * M_PI / 48000.0;
        m_amp = 0.25;
    }

    float generate() {
        m_phase += m_freq * m_multiplier;
        if (m_phase > 2.0f*M_PI) {
            m_phase -= 2.0f * M_PI;
        }
        return m_amp * sin(m_phase);
    }

    float& freq() {
        return m_freq;
    }

    float& amp() {
        return m_amp;
    }

   private:
    float m_freq;
    float m_phase;
    float m_multiplier;
    float m_amp;
};

class AudioEngineImpl : public AudioEngine {
   public:
    AudioEngineImpl();
    ~AudioEngineImpl();
    int init() override;
    void setGraph(AuNodeGraphPtr graph);
    float* amp() override;

   private:
    static void s_dataCallback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount);
    void dataCallback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount);
    ma_device m_device;
    AuNodeGraphPtr m_node_graph;
};

std::unique_ptr<AudioEngine> AudioEngine::create() {
    return std::make_unique<AudioEngineImpl>();
}

AudioEngineImpl::AudioEngineImpl() {
    m_node_graph = 0;
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

    if (ma_device_init(NULL, &config, &m_device) != MA_SUCCESS) {
        return -1;  // Failed to initialize the device.
    }

    ma_device_start(&m_device);  // The device is sleeping by default so you'll need to start it manually.
    return 0;
}

void AudioEngineImpl::setGraph(AuNodeGraphPtr node_graph) {
    m_node_graph = node_graph;
}

float* AudioEngineImpl::amp() {
    return &m_sine.amp();
}


void AudioEngineImpl::s_dataCallback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
    ((AudioEngineImpl*)pDevice->pUserData)->dataCallback(pDevice, pOutput, pInput, frameCount);
}

void AudioEngineImpl::dataCallback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
    float* out = (float*)pOutput;
    for (ma_uint32 i = 0; i < frameCount; ++i) {
        float sample = m_node_graph ? m_node_graph->outputNode()->generate() : 0.0;
        *out++ = sample;
        *out++ = sample;
    }
}
