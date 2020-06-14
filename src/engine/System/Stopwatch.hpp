#pragma once

#include <Util/Prerequisites.hpp>

#include <System/Time.hpp>

namespace engine {

class ENGINE_API Stopwatch {
public:
    Stopwatch();

    Time getElapsedTime() const;

    void start();

    void stop();

    void restart();

private:
    // Return actual time since epoch
    Time getActualTime() const;

    bool m_running;
    Time m_start_time;
    Time m_stop_time;
    Time m_time_buffer;
};

}  // namespace engine
