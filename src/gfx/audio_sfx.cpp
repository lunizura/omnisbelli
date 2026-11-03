#include "gfx/audio_sfx.hpp"

namespace omnisbelli {

bool AudioSFX::enabled = false; // Disabled by default for quiet operation, toggle with [S]
std::queue<AudioEvent> AudioSFX::s_queue;
std::mutex AudioSFX::s_mutex;
std::condition_variable AudioSFX::s_cv;
std::thread AudioSFX::s_worker;
std::atomic<bool> AudioSFX::s_running{false};
std::atomic<bool> AudioSFX::s_initialized{false};

void AudioSFX::initialize() {
    static std::mutex init_mutex;
    std::lock_guard<std::mutex> lock(init_mutex);
    if (s_initialized.load()) return;
    s_running = true;
    s_worker = std::thread(&AudioSFX::worker_thread_func);
    s_initialized = true;
}

void AudioSFX::shutdown() {
    if (!s_initialized.load()) return;
    {
        std::lock_guard<std::mutex> lock(s_mutex);
        s_running = false;
        while (!s_queue.empty()) s_queue.pop();
    }
    s_cv.notify_all();
    if (s_worker.joinable()) {
        s_worker.join();
    }
    s_initialized = false;
}

void AudioSFX::enqueue(AudioEvent evt) {
    if (!enabled) return;
    if (!s_initialized.load()) {
        initialize();
    }
    {
        std::lock_guard<std::mutex> lock(s_mutex);
        if (s_queue.size() < 16) {
            s_queue.push(evt);
        }
    }
    s_cv.notify_one();
}

void AudioSFX::worker_thread_func() {
    while (s_running.load()) {
        AudioEvent evt;
        {
            std::unique_lock<std::mutex> lock(s_mutex);
            s_cv.wait(lock, [] {
                return !s_queue.empty() || !s_running.load();
            });
            if (!s_running.load() && s_queue.empty()) {
                break;
            }
            if (s_queue.empty()) continue;
            evt = s_queue.front();
            s_queue.pop();
        }

        switch (evt) {
        case AudioEvent::RELAY_CLICK:
            ::Beep(1400, 8);
            break;
        case AudioEvent::BREACH_ALARM:
            ::Beep(650, 70);
            ::Beep(450, 90);
            break;
        case AudioEvent::PLASMA_CANNON:
            ::Beep(220, 180);
            break;
        case AudioEvent::EXTERMINATUS_SIREN:
            for (int i = 0; i < 3 && s_running.load(); ++i) {
                ::Beep(880, 120);
                ::Sleep(50);
            }
            break;
        }
    }
}

void AudioSFX::play_relay_click() {
    enqueue(AudioEvent::RELAY_CLICK);
}

void AudioSFX::play_breach_alarm() {
    enqueue(AudioEvent::BREACH_ALARM);
}

void AudioSFX::play_plasma_cannon() {
    enqueue(AudioEvent::PLASMA_CANNON);
}

void AudioSFX::play_exterminatus_siren() {
    enqueue(AudioEvent::EXTERMINATUS_SIREN);
}

void AudioSFX::toggle_sound() {
    enabled = !enabled;
}

// Static cleanup on program exit
static struct AudioCleanup {
    ~AudioCleanup() {
        AudioSFX::shutdown();
    }
} g_audio_cleanup;

} // namespace omnisbelli
