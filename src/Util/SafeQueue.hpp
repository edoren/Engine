#pragma once

#include <Util/Prerequisites.hpp>

template <typename T>
class SafeQueue {
public:
    SafeQueue() {}

    bool IsEmpty() const {
        return m_impl.empty();
    }

    size_t GetSize() const {
        return m_impl.size();
    }

    void Clear() {
        std::lock_guard<std::mutex> lk(mutex_);
        m_impl.clear();
    }

    void Push(const T& value) {
        std::lock_guard<std::mutex> lk(mutex_);
        return m_impl.push_back(value);
    }

    void Push(T&& value) {
        std::lock_guard<std::mutex> lk(mutex_);
        return m_impl.push_back(value);
    }

    template <class... Args>
    void Emplace(Args&&... args) {
        std::lock_guard<std::mutex> lk(mutex_);
        return m_impl.emplace_back(std::forward<Args>(args)...);
    }

    T Pop() {
        T value = m_impl.front();
        m_impl.pop_front();
        return value;
    }

private:
    std::deque<T> m_impl;
    std::mutex mutex_;
};
