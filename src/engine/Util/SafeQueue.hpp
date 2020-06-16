#pragma once

#include <Util/Prerequisites.hpp>

namespace engine {

template <typename T>
class SafeQueue {
public:
    SafeQueue() = default;

    bool isEmpty() const {
        return m_impl.empty();
    }

    size_t getSize() const {
        return m_impl.size();
    }

    void clear() {
        std::lock_guard<std::mutex> lk(m_mutex);
        m_impl.clear();
    }

    void push(const T& value) {
        std::lock_guard<std::mutex> lk(m_mutex);
        return m_impl.push_back(value);
    }

    void push(T&& value) {
        std::lock_guard<std::mutex> lk(m_mutex);
        return m_impl.push_back(value);
    }

    template <class... Args>
    void emplace(Args&&... args) {
        std::lock_guard<std::mutex> lk(m_mutex);
        return m_impl.emplace_back(std::forward<Args>(args)...);
    }

    T pop() {
        T value = m_impl.front();
        m_impl.pop_front();
        return value;
    }

private:
    std::deque<T> m_impl;
    std::mutex m_mutex;
};

}  // namespace engine
