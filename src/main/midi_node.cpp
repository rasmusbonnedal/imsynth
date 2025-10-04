#include "midi_node.h"

#include <imgui.h>

#include <Windows.h>
#include <assert.h>

class MidiDevice {
   public:
    MidiDevice();
    ~MidiDevice();

    float amp() {
        return m_amp;
    }
    float freq() {
        return m_freq;
    }

    static MidiDevice& getInstance();
    struct midi_key_status {
        float amplitude;
        bool is_pressed;
    };

    const midi_key_status* status() const {
        return midi_keys;
    }

   private:
    static void CALLBACK MidiInProc(HMIDIIN hMidiIn, UINT wMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
    void midiInProc(DWORD_PTR dwParam1, DWORD_PTR dwParam2);
    float map_midi_to_freq(BYTE midi_in);
    HMIDIIN hMidiIn;
    float m_freq;
    float m_amp;
    midi_key_status midi_keys[256] = {0};
};

void CALLBACK MidiDevice::MidiInProc(HMIDIIN hMidiIn, UINT wMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2) {
    if (wMsg == MIM_DATA) {
        ((MidiDevice*)dwInstance)->midiInProc(dwParam1, dwParam2);
    }
}

MidiDevice::MidiDevice() {
    UINT numDevices = midiInGetNumDevs();
    if (numDevices == 0) {
        printf("hepp\n");
    } else if (midiInOpen(&hMidiIn, 0, (DWORD_PTR)MidiInProc, (DWORD_PTR)this, CALLBACK_FUNCTION) != MMSYSERR_NOERROR) {
        printf("Failed to open MIDI input device!");
    }
    midiInStart(hMidiIn);
}

MidiDevice::~MidiDevice() {
    midiInStop(hMidiIn);
    midiInClose(hMidiIn);
}

MidiDevice& MidiDevice::getInstance() {
    static MidiDevice midi;
    return midi;
}

void MidiDevice::midiInProc(DWORD_PTR dwParam1, DWORD_PTR dwParam2) {
    BYTE status = dwParam1 & 0xFF;
    BYTE data1 = (dwParam1 >> 8) & 0xFF;
    BYTE data2 = (dwParam1 >> 16) & 0xFF;

    if (status == 144) {
        m_freq = map_midi_to_freq(data1);
        m_amp = float(data2) / 127.0;
        midi_keys[data1].amplitude = m_amp;
        midi_keys[data1].is_pressed = true;
    }
    if (status == 128) {
        m_amp = 0.0f;
        midi_keys[data1].is_pressed = false;
        midi_keys[data1].amplitude = 0;
    }

    // std::cout << "MIDI Message: Status=" << (int)status << ", Data1=" << (int)data1 << ", Data2=" << (int)data2 << std::endl;
    printf("MIDI Message: Status= %x, Data1= %x, Data2= %x, \n", status, data1, data2);
    printf("new freq %f \n", m_freq);
}

float MidiDevice::map_midi_to_freq(BYTE midi_in) {
    float a = ((float)midi_in - 69.0) / 12.0;
    float f = 440.0 * pow(2.0, a);
    return f;
}

AuMidiSource::AuMidiSource() {
    addOutPin("amp");
    addOutPin("freq");
    MidiDevice::getInstance();
}

float AuMidiSource::generate(size_t index) {
    assert(index < outPins());
    if (index == 0) {
        return MidiDevice::getInstance().amp();
    } else if (index == 1) {
        return MidiDevice::getInstance().freq();
    }
    return 0.0f;
}

std::unique_ptr<ImguiWindow> MidiWindow::create() {
    return std::make_unique<MidiWindow>();
}

void MidiWindow::frame() {
    ImGui::Begin("Input stats");
    MidiDevice& midi = MidiDevice::getInstance();
    float total_amp = 0.0f;
    int nof_keys_pressed = 0;
    for (int b_idx = 0; b_idx < 128; b_idx++) {
        float amp = midi.status()[b_idx].amplitude;
        total_amp += amp;
        float r = amp * 0.4f + (1.0f - amp) * 0.7f;
        float g = amp * 0.7f + (1.0f - amp) * 0.4f;
        float b = 0.2f;
        /*
        if (midi_keys[b_idx].is_pressed) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.4f, 0.7f, 0.2f, 1.0f));
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.4f, 0.2f, 1.0f));
        }
        */
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(r, g, b, 1.0f));

        ImGui::Button(std::to_string(b_idx).c_str(), ImVec2(32, 32));
        if ((b_idx + 1) % 8 != 0) {
            ImGui::SameLine();
        }
        ImGui::PopStyleColor(1);

        if (midi.status()[b_idx].is_pressed) {
            nof_keys_pressed++;
        }
    }
    ImGui::End();
}
