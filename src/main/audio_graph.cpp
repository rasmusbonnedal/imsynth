#include "audio_graph.h"
#include "midi_node.h"
#include <chrono>


#define STB_HEXWAVE_IMPLEMENTATION
#include "stb_hexwave.h"


#include <assert.h>

size_t AuNodeBase::inPins() {
    return m_in_pins.size();
}

Pin& AuNodeBase::inPin(size_t index) {
    return m_in_pins[index];
}

size_t AuNodeBase::outPins() {
    return m_out_pins.size();
}

Pin& AuNodeBase::outPin(size_t index) {
    return m_out_pins[index];
}

void AuNodeBase::addInPin(const std::string& name, float value) {
    m_in_pins.emplace_back(name, value);
}

void AuNodeBase::addOutPin(const std::string& name) {
    m_out_pins.emplace_back(name, 0.0f);
}

AuJitterGenerator::AuJitterGenerator() {
    addInPin("input", 1.0);
    addInPin("jitter", 0.1);
    addOutPin("out");

}

float AuJitterGenerator::generate(size_t index) {
    float input = inPin(0).generate();
    float jitter_factor = inPin(1).generate()*input;

    float r = rand() / (float)RAND_MAX;

    return ((jitter_factor * r)-jitter_factor*0.5) + input;

}

AuSineGenerator::AuSineGenerator() {
    addInPin("frequency", 440);
    addInPin("amplitude", 1);
    addOutPin("out");
    m_phase = 0;
    m_multiplier = 2.0 * M_PI / 48000.0;
}

float AuSineGenerator::generate(size_t index) {
    assert(index < outPins());
    m_phase += inPin(0).generate() * m_multiplier;
    while (m_phase > (2 * M_PI)) {
        m_phase -= (2 * M_PI);
    }
    return inPin(1).generate() * sin(m_phase);
}

AuEMAGenerator::AuEMAGenerator() {
    addInPin("in", 0);
    addInPin("alpha", 0.9);
    addOutPin("out");
    m_alpha = 0.9;
    m_previous = 0.0;
}

float AuEMAGenerator::generate(size_t index) {
    float current = inPin(0).generate();
    m_alpha = inPin(1).generate();
    float out = m_alpha * current + (1.0 - m_alpha) * m_previous;
    m_previous = out;
    return out;
}

namespace {
void sawtooth(int& reflect, float& time, float& height, float& wait) {
    reflect = 1;
    time = 1;
    height = 0.5;
    wait = 0;
}
void triangle(int& reflect, float& time, float& height, float& wait) {
    reflect = 1;
    time = 0.5;
    height = 0;
    wait = 0;
}
void square(int& reflect, float& time, float& height, float& wait) {
    reflect = 1;
    time = 0;
    height = 1;
    wait = 0;
}
void stairs(int& reflect, float& time, float& height, float& wait) {
    reflect = 0;
    time = 0;
    height = 1;
    wait = 0.5;
}
}  // namespace

AuHexGenerator::AuHexGenerator() {
    addInPin("frequency", 440);
    addInPin("amplitude", 1);
    addInPin("type", 0);
    addOutPin("out");
    //                     reflect   time   height   wait
    //      Sawtooth          1       1      any      0
    //      Sawtooth (8va)    1       0      -1       0
    //      Triangle          1       0.5     0       0
    //      Square            1       0       1       0
    //      Square            0       0       1       0
    //      Triangle          0       0.5     0       0
    //      Triangle          0       0      -1       0
    //      AlternatingSaw    0       0       0       0
    //      AlternatingSaw    0       1      any      0
    //      Stairs            0       0       1       0.5

    hexwave_init(32, 16, NULL);
    m_osc = new HexWave;
    int reflect_flag = 1;
    float peak_time = 1.0f;
    float half_height = 0.5f;
    float zero_wait = 0.0f;
    sawtooth(reflect_flag, peak_time, half_height, zero_wait);
    hexwave_create(m_osc, reflect_flag, peak_time, half_height, zero_wait);
    m_wave_type = 0;
}

AuHexGenerator::~AuHexGenerator() {
    delete m_osc;
}

float AuHexGenerator::generate(size_t index) {
    assert(index < outPins());
    int wave_type = (int)inPin(2).generate();
    if (wave_type != m_wave_type) {
        m_wave_type = wave_type;
        int reflect;
        float time, height, wait;
        switch (m_wave_type) {
            case 0:
                sawtooth(reflect, time, height, wait);
                break;
            case 1:
                triangle(reflect, time, height, wait);
                break;
            case 2:
                square(reflect, time, height, wait);
                break;
            default:
                stairs(reflect, time, height, wait);
                break;
        }
        hexwave_change(m_osc, reflect, time, height, wait);
    }
    // If this is smaller than this hexwave writes beyond the buffer with triangle
    // wave
    float samples[16];
    hexwave_generate_samples(samples, 1, m_osc, inPin(0).generate() / 48000.0f);
    return inPin(1).generate() * samples[0];
}

AuSub::AuSub() {
    addInPin("in1", 0);
    addInPin("in2", 0);
    addOutPin("out");
}

float AuSub::generate(size_t index) {
    assert(index < outPins());
    return inPin(0).generate() - inPin(1).generate();
}

AuADSR::AuADSR() : m_t(0), m_last(-1), m_r(0) {
    addInPin("amplitude", 1);
    addInPin("A", 0.1);
    addInPin("D", 0.4);
    addInPin("S", 0.6);
    addInPin("R", 0.8);
    addOutPin("out");
}

namespace {
float calcADS(float t, float A, float D, float S) {
    if (t <= A) { // We're in attack phase, raise until t == A
        float a = (t / A);
        return a;
    }
    t -= A;
    if (t <= D) { // We're in decay phase, lower until t == A + D
        float d = (1 - t / D);
        float a = S + d * (1 - S);
        return a;
    }
    return S; // We're in sustain phase, return S while note on
}
}  // namespace

class DePopper {
   public:
    float value(float value) {
        const float delta = 0.01f;
        float diff = abs(value - m_last);
        if (diff < delta) {
            m_last = value;
        } else {
            if (value < m_last) {
                m_last = m_last - delta;
            } else {
                m_last = m_last + delta;
            }
        }
        return sqrt(m_last);
    }

   private:
    float m_last = 0.0f;
};


float AuADSR::generate(size_t index) {
    static DePopper de_popper;
    const float amplitude = inPin(0).generate();
    const float A = inPin(1).generate();
    const float D = inPin(2).generate();
    const float S = std::min(inPin(3).generate(), 1.0f);
    const float R = inPin(4).generate();

    float ads = calcADS(m_t, A, D, S);
    m_t += 1.0 / 48000.0;
    // Assume note change when amplitude change
    if (amplitude != m_last) {
        // Assume note off, start release phase from current value
        if (amplitude == 0) {
            m_r = ads * m_last;
            m_rc = m_r / (R * 48000.0f);
        }
        if (amplitude != 0 || m_r == 0) {
            m_t = 0;
            ads = calcADS(m_t, A, D, S);
            m_r = 0;
        }
        m_last = amplitude;
    }
    if (m_r > 0) {
        m_r = std::max(0.0f, m_r - m_rc);
        return de_popper.value(m_r);
    }
    return de_popper.value(amplitude * ads);
}

AuNodeGraphPtr createTestGraph() {
    AuNodeGraphPtr node_graph = std::make_shared<AuNodeGraph>();

    AuNodePtr midi1 = std::make_shared<AuMidiSource>();
    node_graph->addNode(midi1);

    auto hexwave = std::make_shared<AuHexGenerator>();
    node_graph->addNode(hexwave);
    hexwave->inPin(0).connect(midi1, 1);

    AuNodePtr sub = std::make_shared<AuSub>();
    node_graph->addNode(sub);
    sub->inPin(0).connect(hexwave, 0);
    node_graph->setOutputNode(sub);


    AuNodePtr jitter = std::make_shared<AuJitterGenerator>();
    node_graph->addNode(jitter);

    AuNodePtr EWA = std::make_shared<AuEMAGenerator>();
    node_graph->addNode(EWA);

    AuNodePtr repeat = std::make_shared<AuMidiRepeater>();
    node_graph->addNode(repeat);
    
    auto adsr = std::make_shared<AuADSR>();
    adsr->inPin(0).connect(midi1, 0);
    adsr->inPin(1).set(0.1);  // A
    adsr->inPin(2).set(0.3);  // D
    adsr->inPin(3).set(0.1);  // S
    adsr->inPin(4).set(0.2);  // R
    node_graph->addNode(adsr);
    hexwave->inPin(1).connect(adsr, 0);

    return node_graph;
}
