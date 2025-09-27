#include "audio_graph.h"
#include "midi_node.h"

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

AuHexGenerator::AuHexGenerator() {
    addInPin("frequency", 440);
    addInPin("amplitude", 1);
    addOutPin("out");
    m_phase = 0;
    m_multiplier = 2.0 * M_PI / 48000.0;

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


    hexwave_create((HexWave*)m_osc, reflect_flag, peak_time, half_height, zero_wait);

    
}

float AuHexGenerator::generate(size_t index) {
    assert(index < outPins());
    m_phase += inPin(0).generate() * m_multiplier;
    
    while (m_phase > (2 * M_PI)) {
        m_phase -= (2 * M_PI);
    }
    hexwave_generate_samples(&m_samples[0], 1024, (HexWave*)m_osc, inPin(0).generate() / 48000.0f);

    int idx = (int)((m_phase / (2 * M_PI)) * 1024.0f);

    return inPin(1).generate() * m_samples[idx];
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

AuNodeGraphPtr createTestGraph() {
    AuNodeGraphPtr node_graph = std::make_shared<AuNodeGraph>();
    AuNodePtr midi1 = std::make_shared<AuMidiSource>();
    node_graph->addNode(midi1);
    AuNodePtr sin1 = std::make_shared<AuSineGenerator>();
    sin1->inPin(0).set(440);
    sin1->inPin(1).set(0.8);
    node_graph->addNode(sin1);
    AuNodePtr sin2 = std::make_shared<AuHexGenerator>();
    sin2->inPin(0).set(100);
    sin2->inPin(1).set(0.8);
    node_graph->addNode(sin2);
    AuNodePtr sub = std::make_shared<AuSub>();
    sub->inPin(0).connect(sin1, 0);
    sub->inPin(1).connect(sin2, 0);
    node_graph->addNode(sub);
    node_graph->setOutputNode(sub);
    return node_graph;
}
