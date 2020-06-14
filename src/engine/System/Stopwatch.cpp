#include <System/Stopwatch.hpp>

namespace engine {

Stopwatch::Stopwatch()
      : m_running(false),
        m_start_time(getActualTime()),
        m_stop_time(m_start_time),
        m_time_buffer(Time::sZero) {}

Time Stopwatch::getElapsedTime() const {
    Time duration;
    if (m_running) {
        duration = getActualTime() - m_start_time - m_time_buffer;
    } else {
        duration = m_stop_time - m_start_time - m_time_buffer;
    }
    return duration;
}

void Stopwatch::start() {
    if (!m_running) {
        m_time_buffer += getActualTime() - m_stop_time;
        m_running = true;
    }
}

void Stopwatch::stop() {
    if (m_running) {
        m_stop_time = getActualTime();
        m_running = false;
    }
}

void Stopwatch::restart() {
    Time now = getActualTime();
    m_start_time = now;
    m_stop_time = now;
    m_time_buffer = Time::sZero;
}

Time Stopwatch::getActualTime() const {
    auto now = std::chrono::steady_clock::now().time_since_epoch();
    int64 nanoseconds = std::chrono::nanoseconds(now).count();
    return Time::FromNanoseconds(nanoseconds);
}

}  // namespace engine
