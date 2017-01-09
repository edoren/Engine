#pragma once

#include <chrono>

#include <System/Time.hpp>
#include <Util/Precompiled.hpp>

namespace engine {

class Stopwatch {
public:
    Stopwatch()
          : running_(false),
            start_time_(GetActualTime()),
            stop_time_(start_time_),
            time_buffer_(Time::ZERO) {}

    Time GetElapsedTime() const {
        Time duration;
        if (running_) {
            duration = GetActualTime() - start_time_ - time_buffer_;
        } else {
            duration = stop_time_ - start_time_ - time_buffer_;
        }
        return duration;
    }

    void Start() {
        if (!running_) {
            time_buffer_ += GetActualTime() - stop_time_;
            running_ = true;
        }
    }

    void Stop() {
        if (running_) {
            stop_time_ = GetActualTime();
            running_ = false;
        }
    }

    void Restart() {
        Time now = GetActualTime();
        start_time_ = now;
        stop_time_ = now;
        time_buffer_ = Time::ZERO;
    }

private:
    // return actual time since epoch
    Time GetActualTime() const {
        auto now = std::chrono::steady_clock::now().time_since_epoch();
        int64 nanoseconds = std::chrono::nanoseconds(now).count();
        return Time::FromNanoseconds(nanoseconds);
    };

private:
    bool running_;
    Time start_time_;
    Time stop_time_;
    Time time_buffer_;
};

}  // namespace engine
