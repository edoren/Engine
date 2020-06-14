#include <Util/Prerequisites.hpp>

#include <Util/AsyncTaskRunner.hpp>

namespace engine {

AsyncTaskRunner::AsyncTaskRunner() : m_is_running(true) {
    auto job = [this]() -> void {
        while (m_is_running) {
            std::unique_lock<std::mutex> lk(m_mutex);
            if (m_work_queue.isEmpty()) {
                m_signaler.wait(lk);
                if (m_work_queue.isEmpty()) {
                    if (!m_is_running) {
                        break;
                    }
                    continue;
                }
            }
            Task task = m_work_queue.pop();
            lk.unlock();
            task();
        }
    };
    auto max_threads = std::thread::hardware_concurrency();
    m_workers.reserve(max_threads);
    for (decltype(max_threads) i = 0; i < max_threads; i++) {
        m_workers.emplace_back(job);
    }
}

AsyncTaskRunner::~AsyncTaskRunner() {
    m_is_running = false;
    m_signaler.notify_all();
    for (auto& worker : m_workers) {
        worker.join();
    }
}

void AsyncTaskRunner::execute(Task&& f) {
    m_work_queue.push(std::move(f));
    m_signaler.notify_one();
}

}  // namespace engine
