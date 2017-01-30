#include <System/Stopwatch.hpp>

namespace engine {

Stopwatch::Stopwatch()
      : running_(false),
        start_time_(GetActualTime()),
        stop_time_(start_time_),
        time_buffer_(Time::ZERO) {}

Time Stopwatch::GetElapsedTime() const {
    Time duration;
    if (running_) {
        duration = GetActualTime() - start_time_ - time_buffer_;
    } else {
        duration = stop_time_ - start_time_ - time_buffer_;
    }
    return duration;
}

void Stopwatch::Start() {
    if (!running_) {
        time_buffer_ += GetActualTime() - stop_time_;
        running_ = true;
    }
}

void Stopwatch::Stop() {
    if (running_) {
        stop_time_ = GetActualTime();
        running_ = false;
    }
}

void Stopwatch::Restart() {
    Time now = GetActualTime();
    start_time_ = now;
    stop_time_ = now;
    time_buffer_ = Time::ZERO;
}

Time Stopwatch::GetActualTime() const {
    auto now = std::chrono::steady_clock::now().time_since_epoch();
    int64 nanoseconds = std::chrono::nanoseconds(now).count();
    return Time::FromNanoseconds(nanoseconds);
};

}  // namespace engine
