#include "main_window.h"

#include <imgui.h>
#include <imgui_node_editor.h>

#include "audio_engine.h"

namespace ed = ax::NodeEditor;

class MainWindow_impl : public MainWindow {
   public:
    MainWindow_impl(AudioEngine& audio);
    ~MainWindow_impl();
    void frame() override;

   private:
    struct LinkInfo {
        ed::LinkId Id;
        ed::PinId InputId;
        ed::PinId OutputId;
    };
    ed::EditorContext* m_context = 0;
    ImVector<LinkInfo> m_links;
    int m_NextLinkId = 100;
    AudioEngine& m_audio;
};

std::unique_ptr<ImguiWindow> MainWindow::create(AudioEngine& audio) {
    return std::make_unique<MainWindow_impl>(audio);
}

MainWindow_impl::MainWindow_impl(AudioEngine& audio) : m_audio(audio) {
    ed::Config config;
    config.SettingsFile = "imsynth-nodeed.json";
    m_context = ed::CreateEditor(&config);
}

MainWindow_impl::~MainWindow_impl() {
    ed::DestroyEditor(m_context);
}

void MainWindow_impl::frame() {
    ImGui::Begin("ImSynth");
    ed::SetCurrentEditor(m_context);
    ed::Begin("My Editor", ImVec2(0.0, 0.0f));
    int uniqueId = 1;

    ed::BeginNode(uniqueId++);
    ImGui::Text("Node A");
    ed::BeginPin(uniqueId++, ed::PinKind::Input);
    ImGui::Text("-> In");
    ed::EndPin();
    ImGui::SameLine();
    ed::BeginPin(uniqueId++, ed::PinKind::Output);
    ImGui::Text("Out ->");
    ed::EndPin();
    ImGui::SetNextItemWidth(80);
    ImGui::DragFloat("Freq", m_audio.freq(), 0.1f, 20, 6000, "%.1f");
    ed::EndNode();

    ed::BeginNode(uniqueId++);
    ImGui::Text("Node B");
    ed::BeginPin(uniqueId++, ed::PinKind::Input);
    ImGui::Text("-> In");
    ed::EndPin();
    ImGui::SameLine();
    ed::BeginPin(uniqueId++, ed::PinKind::Output);
    ImGui::Text("Out ->");
    ed::EndPin();
    ed::EndNode();

    for (auto& linkInfo : m_links) {
        ed::Link(linkInfo.Id, linkInfo.InputId, linkInfo.OutputId);
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
                // ed::AcceptNewItem() return true when user release mouse button.
                if (ed::AcceptNewItem()) {
                    // Since we accepted new link, lets add one to our list of links.
                    m_links.push_back({ed::LinkId(m_NextLinkId++), inputPinId, outputPinId});

                    // Draw new link.
                    ed::Link(m_links.back().Id, m_links.back().InputId, m_links.back().OutputId);
                }

                // You may choose to reject connection between these nodes
                // by calling ed::RejectNewItem(). This will allow editor to give
                // visual feedback by changing link thickness and color.
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
                for (auto& link : m_links) {
                    if (link.Id == deletedLinkId) {
                        m_links.erase(&link);
                        break;
                    }
                }
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
