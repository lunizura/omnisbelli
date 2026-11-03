#pragma once
#include <windows.h>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>

namespace omnisbelli {

enum class AudioEvent {
    RELAY_CLICK,
    BREACH_ALARM,
    PLASMA_CANNON,
    EXTERMINATUS_SIREN
};

class AudioSFX {
public:
    static bool enabled;

    static void play_relay_click();
    static void play_breach_alarm();
    static void play_plasma_cannon();
    static void play_exterminatus_siren();
    static void toggle_sound();

    static void initialize();
    static void shutdown();

private:
    static std::queue<AudioEvent> s_queue;
    static std::mutex s_mutex;
    static std::condition_variable s_cv;
    static std::thread s_worker;
    static std::atomic<bool> s_running;
    static std::atomic<bool> s_initialized;

    static void enqueue(AudioEvent evt);
    static void worker_thread_func();
};

} // namespace omnisbelli
