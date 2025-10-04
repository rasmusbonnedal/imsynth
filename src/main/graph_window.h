#pragma once

#include "imgui_window.h"

class AudioEngine;

class GraphWindow : public ImguiWindow {
   public:
    static std::unique_ptr<ImguiWindow> create(AudioEngine& audio);
};
