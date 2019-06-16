#include <Util/Prerequisites.hpp>

#include <Util/AsyncTaskRunner.hpp>

namespace engine {

AsyncTaskRunner::AsyncTaskRunner()
      : m_is_running(true),
        m_work_queue(),
        m_workers(),
        m_signaler(),
        m_mutex() {
    auto job = [this]() -> void {
        while (m_is_running) {
            std::unique_lock<std::mutex> lk(m_mutex);
            if (m_work_queue.IsEmpty()) {
                m_signaler.wait(lk);
                if (m_work_queue.IsEmpty()) {
                    if (!m_is_running)
                        break;
                    else
                        continue;
                }
            }
            Task task = m_work_queue.Pop();
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

void AsyncTaskRunner::Execute(Task&& f) {
    m_work_queue.Push(std::move(f));
    m_signaler.notify_one();
}

}  // namespace engine
