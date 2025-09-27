#include "audio_graph.h"
#include "midi_node.h"

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
    AuNodePtr sin2 = std::make_shared<AuSineGenerator>();
    sin2->inPin(0).set(439);
    sin2->inPin(1).set(0.8);
    node_graph->addNode(sin2);
    AuNodePtr sub = std::make_shared<AuSub>();
    sub->inPin(0).connect(sin1, 0);
    sub->inPin(1).connect(sin2, 0);
    node_graph->addNode(sub);
    node_graph->setOutputNode(sub);
    return node_graph;
}
