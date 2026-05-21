#pragma once
#include <windows.h>
#include <thread>

namespace omnisbelli {

class AudioSFX {
public:
    static bool enabled;

    static void play_relay_click() {
        if (!enabled) return;
        std::thread([]() {
            ::Beep(1400, 8);
        }).detach();
    }

    static void play_breach_alarm() {
        if (!enabled) return;
        std::thread([]() {
            ::Beep(650, 70);
            ::Beep(450, 90);
        }).detach();
    }

    static void play_plasma_cannon() {
        if (!enabled) return;
        std::thread([]() {
            ::Beep(220, 180);
        }).detach();
    }

    static void play_exterminatus_siren() {
        if (!enabled) return;
        std::thread([]() {
            for (int i = 0; i < 3; ++i) {
                ::Beep(880, 120);
                ::Sleep(50);
            }
        }).detach();
    }

    static void toggle_sound() {
        enabled = !enabled;
    }
};

} // namespace omnisbelli
