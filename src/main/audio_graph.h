#pragma once

#define _USE_MATH_DEFINES
#include <math.h>

#include <memory>
#include <string>
#include <string_view>
#include <vector>

class AuNode;
class AuNodeGraph;

using AuNodePtr = std::shared_ptr<AuNode>;
using AuNodeGraphPtr = std::shared_ptr<AuNodeGraph>;

class AuNodeGraph {
   public:
    void addNode(AuNodePtr node) {
        m_nodes.push_back(node);
    }
    void setOutputNode(AuNodePtr node) {
        m_output_node = node;
    }

    AuNodePtr outputNode() {
        return m_output_node;
    }

    const std::vector<AuNodePtr> nodes() const {
        return m_nodes;
    }

   private:
    std::vector<AuNodePtr> m_nodes;
    AuNodePtr m_output_node;
};

class Pin {
   public:
    Pin(const std::string& name, float value) : m_name(name), m_value(value) {}
    float generate() const;

    void set(float value) {
        m_value = value;
    }

    float& value() {
        return m_value;
    }

    void connect(AuNodePtr node) {
        m_connection = node;
    }

    void disconnect() {
        m_connection = 0;
    }

    AuNodePtr node() const {
        return m_connection;
    }

    const std::string& name() const {
        return m_name;
    }

   private:
    std::string m_name;
    float m_value;
    AuNodePtr m_connection;
};

class AuNode {
   public:
    virtual ~AuNode() {}
    virtual float generate() = 0;
    virtual size_t inPins() = 0;
    virtual Pin& inPin(size_t index) = 0;
    virtual std::string_view name() const = 0;
};

class AuNodeBase : public AuNode {
   public:
    size_t inPins() override;
    Pin& inPin(size_t index) override;

    void addInPin(const std::string& name, float value);

   protected:
    std::vector<Pin> m_in_pins;
};

inline float Pin::generate() const {
    return m_connection ? m_connection->generate() : m_value;
}

class AuSineGenerator : public AuNodeBase {
   public:
    AuSineGenerator();
    float generate() override;
    std::string_view name() const {
        return "SineGenerator";
    }

   private:
    float m_phase;
    float m_multiplier;
};

class AuSub : public AuNodeBase {
   public:
    AuSub();
    float generate() override;
    std::string_view name() const {
        return "Sub";
    }
};

AuNodeGraphPtr createTestGraph();
