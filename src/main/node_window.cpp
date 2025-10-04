#include "node_window.h"

#include "audio_engine.h"

#include <imgui.h>

class NodeWindow_impl : public NodeWindow {
   public:
    NodeWindow_impl(AudioEngine& audio_engine);
    ~NodeWindow_impl();
    void frame() override;

   private:
    AudioEngine& m_audio;
};

std::unique_ptr<NodeWindow> NodeWindow::create(AudioEngine& audio_engine) {
    return std::make_unique<NodeWindow_impl>(audio_engine);
}

NodeWindow_impl::NodeWindow_impl(AudioEngine& audio_engine) : m_audio(audio_engine) {
}

NodeWindow_impl::~NodeWindow_impl() {

}

void NodeWindow_impl::frame() {
    ImGui::Begin("Nodes");
    if (ImGui::Button("Sine")) m_audio.getGraph()->addNode(std::make_shared<AuSineGenerator>());
    if (ImGui::Button("Hex")) m_audio.getGraph()->addNode(std::make_shared<AuHexGenerator>());
    if (ImGui::Button("Sub")) m_audio.getGraph()->addNode(std::make_shared<AuSub>());
    ImGui::End();
}
