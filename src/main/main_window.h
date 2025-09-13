#pragma once

#include "imgui_window.h"

class AudioEngine;

class MainWindow : public ImguiWindow {
   public:
    static std::unique_ptr<ImguiWindow> create(AudioEngine& audio);
};
