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

    bool m_running;
    Time m_start_time;
    Time m_stop_time;
    Time m_time_buffer;
};

}  // namespace engine
