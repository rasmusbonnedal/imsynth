#pragma once
#include <memory>
#include <vector>

class ImguiWindow {
   public:
    virtual ~ImguiWindow() {}
    virtual void frame() = 0;
};

using ImguiWindowVector = std::vector<std::unique_ptr<ImguiWindow>>;
