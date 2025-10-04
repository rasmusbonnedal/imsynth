#include "main_window.h"

#include <imgui.h>
#include <imgui_node_editor.h>

#include <format>
#include <iostream>
#include <map>
#include <string>

#include "audio_engine.h"

namespace ed = ax::NodeEditor;

class EdIdMapper {
   public:
    EdIdMapper() : m_next_free_index(100) {}

    ed::NodeId getNodeId(AuNodePtr node) {
        return (ed::NodeId)get(pt(node, 0, Node));
    }

    ed::PinId getInPinId(AuNodePtr node, int index) {
        return (ed::PinId)get(pt(node, index, InPin));
    }

    ed::PinId getOutPinId(AuNodePtr node, int index) {
        return (ed::PinId)get(pt(node, index, OutPin));
    }

    ed::LinkId getLinkId(AuNodePtr node, int input_index) {
        return (ed::LinkId)get(pt(node, input_index, Link));
    }

    AuNodePtr getNode(ed::NodeId id) {
        assert(m_id_to_ptr.count((size_t)id) > 0);
        auto p = m_id_to_ptr[(size_t)id];
        assert(p.second.second == Node);
        return p.first;
    }

    std::pair<AuNodePtr, int> getInPin(ed::PinId id) {
        assert(m_id_to_ptr.count((size_t)id) > 0);
        auto p = m_id_to_ptr[(size_t)id];
        assert(p.second.second == InPin);
        return {p.first, p.second.first};
    }

    std::pair<AuNodePtr, int> getOutPin(ed::PinId id) {
        assert(m_id_to_ptr.count((size_t)id) > 0);
        auto p = m_id_to_ptr[(size_t)id];
        assert(p.second.second == OutPin);
        return {p.first, p.second.first};
    }

    std::pair<AuNodePtr, int> getLink(ed::LinkId id) {
        assert(m_id_to_ptr.count((size_t)id) > 0);
        auto p = m_id_to_ptr[(size_t)id];
        assert(p.second.second == Link);
        return {p.first, p.second.first};
    }

    bool isInPin(ed::PinId id) const {
        return m_id_to_ptr.count((size_t)id) > 0 && m_id_to_ptr.at((size_t)id).second.second == InPin;
    }

    bool isOutPin(ed::PinId id) const {
        return m_id_to_ptr.count((size_t)id) > 0 && m_id_to_ptr.at((size_t)id).second.second == OutPin;
    }

   private:
    enum Type { Node, InPin, OutPin, Link };
    using PtrType = std::pair<AuNodePtr, std::pair<int, Type>>;
    PtrType pt(AuNodePtr node, int index, Type type) {
        return {node, {index, type}};
    }

    size_t get(PtrType p) {
        if (m_ptr_to_id.count(p) == 0) {
            size_t id = m_next_free_index++;
            m_ptr_to_id[p] = id;
            m_id_to_ptr[id] = p;
            return id;
        } else {
            return m_ptr_to_id[p];
        }
    }

    std::map<PtrType, size_t> m_ptr_to_id;
    std::map<size_t, PtrType> m_id_to_ptr;
    size_t m_next_free_index;
};

class MainWindow_impl : public MainWindow {
   public:
    MainWindow_impl(AudioEngine& audio);
    ~MainWindow_impl();
    void frame() override;

   private:
    ed::EditorContext* m_context = 0;
    int m_NextLinkId = 100;
    AudioEngine& m_audio;
    AuNodeGraphPtr m_node_graph;
    EdIdMapper m_id_mapper;
};

std::unique_ptr<ImguiWindow> MainWindow::create(AudioEngine& audio) {
    return std::make_unique<MainWindow_impl>(audio);
}

MainWindow_impl::MainWindow_impl(AudioEngine& audio) : m_audio(audio) {
    ed::Config config;
    config.SettingsFile = "imsynth-nodeed.json";
    m_context = ed::CreateEditor(&config);
    m_node_graph = createTestGraph();
    m_audio.setGraph(m_node_graph);
}

MainWindow_impl::~MainWindow_impl() {
    ed::DestroyEditor(m_context);
}

// #define DEBUG_PINS

void MainWindow_impl::frame() {
    ImGui::Begin("ImSynth");
    ed::SetCurrentEditor(m_context);
    ed::Begin("My Editor", ImVec2(0.0, 0.0f));

    int links = 0;
    for (const auto& node : m_node_graph->nodes()) {
        ImGui::PushID(node.get());
        ed::NodeId node_id = m_id_mapper.getNodeId(node);
        ed::BeginNode(node_id);
#if defined(DEBUG_PINS)
        ImGui::Text(std::format("{} ({})", node->name(), (size_t)node_id).c_str());
#else
        ImGui::Text(std::string(node->name()).c_str());
#endif
        if (node == m_node_graph->getOutputNode()) {
            ImGui::Text("Output: %.0f dB", m_audio.getDb());
        }

        for (size_t i = 0; i < std::max(node->outPins(), node->inPins()); ++i) {
            if (i < node->inPins()) {
                ed::PinId in_pin = m_id_mapper.getInPinId(node, i);
                ed::BeginPin(in_pin, ed::PinKind::Input);
                Pin& inpin = node->inPin(i);
#if defined(DEBUG_PINS)
                ImGui::Text(std::format("{} ({})", node->inPin(i).name(), (size_t)in_pin).c_str());
#else
                ImGui::Text(inpin.name().c_str());
#endif
                ed::EndPin();
                ImGui::SameLine();
                ImGui::SetNextItemWidth(50);
                ImGui::PushID(i);
                if (inpin.node()) {
                    // ImGui::Text("%.1f", inpin.generate());
                } else {
                    ImGui::DragFloat("", &(node->inPin(i).value()), 0.1, 0, 100, "%.1f");
                }
                ImGui::PopID();
            } else {
                ImGui::Text("                   ");
            }
            if (i < node->outPins()) {
                ImGui::SameLine();
                ed::PinId out_pin = m_id_mapper.getOutPinId(node, i);
                ed::BeginPin(out_pin, ed::PinKind::Output);
#if defined(DEBUG_PINS)
                ImGui::Text(std::format("{} ({})", node->outPin(i).name(), (size_t)out_pin).c_str());
#else
                ImGui::Text(node->outPin(i).name().c_str());
#endif
                ed::EndPin();
            }
        }
        ed::EndNode();
        ImGui::PopID();
    }
    for (const auto& node : m_node_graph->nodes()) {
        for (size_t pin = 0; pin < node->inPins(); ++pin) {
            if (AuNodePtr upstream = node->inPin(pin).node()) {
                ed::LinkId link_id = m_id_mapper.getLinkId(node, pin);
                ed::PinId in_pin_id = m_id_mapper.getInPinId(node, pin);
                ed::PinId out_pin_id = m_id_mapper.getOutPinId(upstream, node->inPin(pin).index());
                ed::Link(link_id, in_pin_id, out_pin_id);
            }
        }
    }

    if (ed::BeginCreate()) {
        ed::PinId inputPinId, outputPinId;
        if (ed::QueryNewLink(&inputPinId, &outputPinId)) {
            // QueryNewLink returns true if editor want to create new link between pins.
            //
            // Link can be created only for two valid pins, it is up to you to
            // validate if connection make sense. Editor is happy to make any.
            //
            // Link always goes from input to output. User may choose to drag
            // link from output pin or input pin. This determine which pin ids
            // are valid and which are not:
            //   * input valid, output invalid - user started to drag new ling from input pin
            //   * input invalid, output valid - user started to drag new ling from output pin
            //   * input valid, output valid   - user dragged link over other pin, can be validated

            if (inputPinId && outputPinId)  // both are valid, let's accept link
            {
                bool is_ok = false;
                if (m_id_mapper.isOutPin(inputPinId) && m_id_mapper.isInPin(outputPinId)) {
                    std::swap(inputPinId, outputPinId);
                }
                if (m_id_mapper.isOutPin(outputPinId) && m_id_mapper.isInPin(inputPinId)) {
                    is_ok = true;
                }
                // ed::AcceptNewItem() return true when user release mouse button.
                if (is_ok) {
                    if (ed::AcceptNewItem()) {
                        auto inpin = m_id_mapper.getInPin(inputPinId);
                        auto outpin = m_id_mapper.getOutPin(outputPinId);
                        if (inpin.first != outpin.first) {
                            inpin.first->inPin(inpin.second).connect(outpin.first, outpin.second);
                            // Draw new link.
                            ed::Link(m_id_mapper.getLinkId(inpin.first, inpin.second), inputPinId, outputPinId);
                        }
                    }
                } else {
                    ed::RejectNewItem();
                }
            }
        }
        ed::EndCreate();  // Wraps up object creation action handling.
    }

    // Handle deletion action
    if (ed::BeginDelete()) {
        // There may be many links marked for deletion, let's loop over them.
        ed::LinkId deletedLinkId;
        while (ed::QueryDeletedLink(&deletedLinkId)) {
            // If you agree that link can be deleted, accept deletion.
            if (ed::AcceptDeletedItem()) {
                // Then remove link from your data.
                auto link = m_id_mapper.getLink(deletedLinkId);
                link.first->inPin(link.second).disconnect();
            }
            // You may reject link deletion by calling:
            // ed::RejectDeletedItem();
        }
        ed::EndDelete();  // Wrap up deletion action
    }

    ed::End();
    ed::SetCurrentEditor(nullptr);

    ImGui::End();
}
