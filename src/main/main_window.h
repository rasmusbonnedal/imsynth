#pragma once

#include "imguiwindow.h"

class MainWindow : public ImguiWindow {
   public:
    static std::unique_ptr<ImguiWindow> create();
};
