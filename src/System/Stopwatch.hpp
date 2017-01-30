#pragma once

#include <Util/Prerequisites.hpp>

#include <System/Time.hpp>

namespace engine {

class ENGINE_API Stopwatch {
public:
    Stopwatch();

    Time GetElapsedTime() const;

    void Start();

    void Stop();

    void Restart();

private:
    // Return actual time since epoch
    Time GetActualTime() const;

private:
    bool running_;
    Time start_time_;
    Time stop_time_;
    Time time_buffer_;
};

}  // namespace engine
