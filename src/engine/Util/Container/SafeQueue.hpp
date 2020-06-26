#pragma once

#include <Util/Prerequisites.hpp>

#include <deque>
#include <mutex>

namespace engine {

template <typename T>
class SafeQueue {
public:
    SafeQueue() = default;

    bool isEmpty() const;

    size_t getSize() const;

    void clear();

    void push(const T& value);

    void push(T&& value);

    template <class... Args>
    void emplace(Args&&... args);

    T pop();

private:
    std::deque<T> m_impl;
    std::mutex m_mutex;
};

}  // namespace engine

#include <Util/Container/SafeQueue.inl>
