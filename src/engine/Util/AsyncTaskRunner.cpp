#include <Util/Prerequisites.hpp>

#include <Util/AsyncTaskRunner.hpp>

#include <mutex>
#include <thread>

namespace engine {

AsyncTaskRunner::AsyncTaskRunner() : m_isRunning(true) {
    auto job = [this]() -> void {
        while (m_isRunning) {
            std::unique_lock<std::mutex> lk(m_mutex);
            if (m_workQueue.isEmpty()) {
                m_signaler.wait(lk);
                if (m_workQueue.isEmpty()) {
                    if (!m_isRunning) {
                        break;
                    }
                    continue;
                }
            }
            Task task = m_workQueue.pop();
            lk.unlock();
            task();
        }
    };
    auto maxThreads = std::thread::hardware_concurrency();
    m_workers.reserve(maxThreads);
    for (decltype(maxThreads) i = 0; i < maxThreads; i++) {
        m_workers.emplace_back(job);
    }
}

AsyncTaskRunner::~AsyncTaskRunner() {
    m_isRunning = false;
    m_signaler.notify_all();
    for (auto& worker : m_workers) {
        worker.join();
    }
}

void AsyncTaskRunner::execute(Task&& f) {
    m_workQueue.push(std::move(f));
    m_signaler.notify_one();
}

}  // namespace engine
