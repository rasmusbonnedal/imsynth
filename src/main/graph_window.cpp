#include "graph_window.h"

#include <format>

#include <imgui.h>

#include "audio_engine.h"

class GraphWindow_impl : public GraphWindow {
   public:
    GraphWindow_impl(AudioEngine& audio);
    ~GraphWindow_impl();
    void frame() override;

   private:
    AudioEngine& m_audio;
};

std::unique_ptr<ImguiWindow> GraphWindow::create(AudioEngine& audio) {
    return std::make_unique<GraphWindow_impl>(audio);
}

GraphWindow_impl::GraphWindow_impl(AudioEngine& audio) : m_audio(audio) {

}

GraphWindow_impl::~GraphWindow_impl() {}

// #define DEBUG_PINS

namespace {
void oscilloscope(const AudioEngine& audio, float time_scale, int& pos, int& len) {
    auto history = audio.getHistory();
    len = time_scale * 48000.0f;
    pos = audio.getHistoryPos() - len;

    while (pos < 0) {
        pos += history.size();
    }
    float cur = history.at(pos);
    int zeros_to_find = 8;
    for (int i = 0; i < 4800; ++i) {
        if (--pos < 0) {
            pos += history.size();
        }
        float prev = history.at(pos);
        bool zero_crossing = prev < 0 && cur > 0;
        cur = prev;
        if (zero_crossing && --zeros_to_find == 0) {
            return;
        }
    }
}

void current(const AudioEngine& audio, float time_scale, int& pos, int& len) {
    auto history = audio.getHistory();
    len = time_scale * 48000.0f;
    pos = audio.getHistoryPos() - len * 8;
}

void all(const AudioEngine& audio, int& pos, int& len) {
    auto history = audio.getHistory();
    len = history.size() / 16;
    pos = 0;
}

struct vector_values_getter_data {
    std::vector<float>& vec;
    size_t pos;
};

float vector_values_getter(void* data, int idx) {
    const auto& vec_data = *(vector_values_getter_data*)data;
    return vec_data.vec.at((idx + vec_data.pos) % vec_data.vec.size());
}

}  // namespace

void GraphWindow_impl::frame() {
    ImGui::Begin("Graph");
    auto history = m_audio.getHistory();

    static float time_scale = 0.01f;
    static int type = 0;

    int pos, len;
    if (type == 0) {
        oscilloscope(m_audio, time_scale, pos, len);
    } else if (type == 1) {
        current(m_audio, time_scale, pos, len);
    } else {
        all(m_audio, pos, len);
    }
    while (pos < 0) {
        pos += history.size();
    }
    auto region = ImGui::GetContentRegionAvail();
    region.y -= 40;
    vector_values_getter_data data = {history, (size_t)pos};
    ImGui::PlotLines("##a", &vector_values_getter, &data, len, 0, std::format("{}", pos).c_str(), -1, 1, region);
    ImGui::SetNextItemWidth(80);
    ImGui::BeginDisabled(type >= 2);
    ImGui::DragFloat("Time Scale", &time_scale, 0.001f, 0.001f, 1.0f);
    ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::SetNextItemWidth(80);
    ImGui::Combo("Type", &type, "Oscilloscope\0Current\0All");
    ImGui::End();
}
