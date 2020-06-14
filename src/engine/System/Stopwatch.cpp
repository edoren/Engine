#include <System/Stopwatch.hpp>

namespace engine {

Stopwatch::Stopwatch()
      : m_running(false),
        m_startTime(getActualTime()),
        m_stopTime(m_startTime),
        m_timeBuffer(Time::sZero) {}

Time Stopwatch::getElapsedTime() const {
    Time duration;
    if (m_running) {
        duration = getActualTime() - m_startTime - m_timeBuffer;
    } else {
        duration = m_stopTime - m_startTime - m_timeBuffer;
    }
    return duration;
}

void Stopwatch::start() {
    if (!m_running) {
        m_timeBuffer += getActualTime() - m_stopTime;
        m_running = true;
    }
}

void Stopwatch::stop() {
    if (m_running) {
        m_stopTime = getActualTime();
        m_running = false;
    }
}

void Stopwatch::restart() {
    Time now = getActualTime();
    m_startTime = now;
    m_stopTime = now;
    m_timeBuffer = Time::sZero;
}

Time Stopwatch::getActualTime() const {
    auto now = std::chrono::steady_clock::now().time_since_epoch();
    int64 nanoseconds = std::chrono::nanoseconds(now).count();
    return Time::FromNanoseconds(nanoseconds);
}

}  // namespace engine
