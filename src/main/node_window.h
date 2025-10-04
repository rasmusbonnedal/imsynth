#pragma once

#include "imgui_window.h"

#include <memory>

class AudioEngine;

class NodeWindow : public ImguiWindow {
   public:
    static std::unique_ptr<NodeWindow> create(AudioEngine& audio_engine);
};
