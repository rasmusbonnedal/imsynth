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

    AuNodePtr getOutputNode() const {
        return m_output_node;
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

    void connect(AuNodePtr node, size_t index) {
        m_connection = node;
        m_index = index;
    }

    void disconnect() {
        m_connection = 0;
    }

    AuNodePtr node() const {
        return m_connection;
    }

    size_t index() const {
        return m_index;
    }

    const std::string& name() const {
        return m_name;
    }

   private:
    std::string m_name;
    float m_value;
    AuNodePtr m_connection;
    size_t m_index;
};

class AuNode {
   public:
    virtual ~AuNode() {}
    virtual float generate(size_t index) = 0;
    virtual size_t inPins() = 0;
    virtual Pin& inPin(size_t index) = 0;
    virtual size_t outPins() = 0;
    virtual Pin& outPin(size_t index) = 0;
    virtual std::string_view name() const = 0;
};

class AuNodeBase : public AuNode {
   public:
    size_t inPins() override;
    Pin& inPin(size_t index) override;
    size_t outPins() override;
    Pin& outPin(size_t index) override;

    void addInPin(const std::string& name, float value);
    void addOutPin(const std::string& name);

   protected:
    std::vector<Pin> m_in_pins;
    std::vector<Pin> m_out_pins;
};

inline float Pin::generate() const {
    return m_connection ? m_connection->generate(m_index) : m_value;
}

class AuSineGenerator : public AuNodeBase {
   public:
    AuSineGenerator();
    float generate(size_t index) override;
    std::string_view name() const {
        return "SineGenerator";
    }

   private:
    float m_phase;
    float m_multiplier;
};

/*
class AuLooper : public AuNodeBase {
   public:
    AuLooper();
    float generate(size_t index) override;
    std::string_view name() const {
        return "Looper";
    }

   private:
    struct sample {
        float amp;
        float freq;
        float start_time;
        float duration;
    };
    sample m_samples[8] = {0};
        
};
*/
class AuEMAGenerator : public AuNodeBase {
   public:
    AuEMAGenerator();
    float generate(size_t index) override;
    std::string_view name() const {
        return "EMAGenerator";
    }

   private:
    float m_previous;
    float m_alpha;
};

class AuJitterGenerator : public AuNodeBase {
   public:
    AuJitterGenerator();
    float generate(size_t index) override;
    std::string_view name() const {
        return "JitterGenerator";
    }

   private:
 };


struct HexWave;

class AuHexGenerator : public AuNodeBase {
   public:
    AuHexGenerator();
    ~AuHexGenerator();
    float generate(size_t index) override;
    std::string_view name() const {
        return "HexGenerator";
    }

   private:
    HexWave* m_osc;
    int m_wave_type;
};

class AuSub : public AuNodeBase {
   public:
    AuSub();
    float generate(size_t index) override;
    std::string_view name() const {
        return "Sub";
    }
};

class AuADSR : public AuNodeBase {
   public:
    AuADSR();
    float generate(size_t index) override;
    std::string_view name() const {
        return "ADSR";
    }
   private:
    float m_t;      // Time since note started
    float m_last;   // Last amplitude to see if note changed
    float m_r;      // Current release amplitude after note off
    float m_rc;     // Amount to subtract from release each sample
};

AuNodeGraphPtr createTestGraph();
