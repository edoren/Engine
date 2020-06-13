#include <System/Stopwatch.hpp>

namespace engine {

Stopwatch::Stopwatch()
      : m_running(false),
        m_start_time(GetActualTime()),
        m_stop_time(m_start_time),
        m_time_buffer(Time::sZero) {}

Time Stopwatch::GetElapsedTime() const {
    Time duration;
    if (m_running) {
        duration = GetActualTime() - m_start_time - m_time_buffer;
    } else {
        duration = m_stop_time - m_start_time - m_time_buffer;
    }
    return duration;
}

void Stopwatch::Start() {
    if (!m_running) {
        m_time_buffer += GetActualTime() - m_stop_time;
        m_running = true;
    }
}

void Stopwatch::Stop() {
    if (m_running) {
        m_stop_time = GetActualTime();
        m_running = false;
    }
}

void Stopwatch::Restart() {
    Time now = GetActualTime();
    m_start_time = now;
    m_stop_time = now;
    m_time_buffer = Time::sZero;
}

Time Stopwatch::GetActualTime() const {
    auto now = std::chrono::steady_clock::now().time_since_epoch();
    int64 nanoseconds = std::chrono::nanoseconds(now).count();
    return Time::FromNanoseconds(nanoseconds);
}

}  // namespace engine
