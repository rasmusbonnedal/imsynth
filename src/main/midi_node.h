#pragma once
#include "audio_graph.h"
#include "imgui_window.h"

class AuMidiSource : public AuNodeBase {
   public:
    AuMidiSource();
    float generate(size_t index) override;
    std::string_view name() const {
        return "MidiIn";
    }
};

class MidiWindow : public ImguiWindow {
   public:
    static std::unique_ptr<ImguiWindow> create();
    void frame() override;
};
