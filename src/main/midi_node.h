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

class AuMidiRepeater : public AuNodeBase {
   public:
    AuMidiRepeater();
    float generate(size_t index) override;
    std::string_view name() const {
        return "MidiRepeat";
    }
    struct note {
        float amp;
        float freq;
        double start;
        double end;
    };
    bool m_start_repeat = false;
    int m_current_record = 0;
    double m_start_playback = -1.0;
    note m_notes[8];
};

class MidiWindow : public ImguiWindow {
   public:
    static std::unique_ptr<ImguiWindow> create();
    void frame() override;
};
