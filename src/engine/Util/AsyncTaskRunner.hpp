#pragma once

#include <Util/Prerequisites.hpp>

#include <Util/Function.hpp>
#include <Util/Container/SafeQueue.hpp>

namespace engine {

class ENGINE_API AsyncTaskRunner {
public:
    using Task = Function<void()>;

    AsyncTaskRunner();

    ~AsyncTaskRunner();

    void execute(Task&& f);

private:
    bool m_isRunning;
    SafeQueue<Task> m_workQueue;
    std::vector<std::thread> m_workers;
    std::condition_variable m_signaler;
    std::mutex m_mutex;
};

}  // namespace engine
