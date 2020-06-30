#pragma once

#include <Util/Prerequisites.hpp>

#include <System/String.hpp>
#include <Util/UTF.hpp>

#include <utility>

namespace engine {

class String;
class StringView;

template <size_t Base,
          typename T,
          typename = std::enable_if_t<std::is_pointer<T>::value && std::is_integral<std::remove_pointer_t<T>>::value>>
class ENGINE_API UTFIterator {
    static_assert((Base == 8) || (Base == 16) || (Base == 32), "Error invalid Base, it should be either 8, 16 or 32");
    static_assert(sizeof(std::remove_pointer_t<T>) * 8 == Base,
                  "Error invalid value (T), it should has the same value in bits as Base");

    friend class String;
    friend class StringView;

public:
    using difference_type = std::ptrdiff_t;
    using value_type = UTFCodeUnitRange<8, T>;
    using pointer = UTFCodeUnitRange<8, T>*;
    using reference = UTFCodeUnitRange<8, T>&;
    using iterator_category = std::forward_iterator_tag;

    UTFIterator() = default;

    UTFIterator(const UTFIterator& other) = default;

    ~UTFIterator() = default;

    UTFIterator& operator=(const UTFIterator& other) = default;

    reference operator*() {
        return m_value;
    }

    pointer operator->() {
        return &m_value;
    }

    constexpr UTFIterator operator+(uint32 num) {
        // Check if already on the end
        auto newBegin = m_value.begin;
        for (uint32 i = 0; i < num; i++) {
            auto it = utf::NextUTF<Base>(newBegin, m_maxRange.second);
            if (it == newBegin) {
                // Error getting next
                return UTFIterator(m_maxRange, m_maxRange.second);
            }
            newBegin = it;
        }
        return UTFIterator(m_maxRange, newBegin);
    }

    UTFIterator& operator+=(uint32 num) {
        *this = *this + num;
        return *this;
    }

    UTFIterator operator++() {
        return *this += 1;
    }

    UTFIterator operator++(int) {
        UTFIterator temp(*this);
        *this += 1;
        return temp;
    }

    bool operator==(const UTFIterator& other) const {
        return m_value == other.m_value && m_maxRange == other.m_maxRange;
    }

    bool operator!=(const UTFIterator& other) const {
        return !(*this == other);
    }

    void swap(UTFIterator& other) {
        std::swap(m_maxRange, other.m_maxRange);
        std::swap(m_value, other.m_value);
    }

private:
    constexpr UTFIterator(std::pair<T, T> maxRange, T begin)
          : m_maxRange(std::move(maxRange)),
            m_value(begin, m_maxRange) {}

    std::pair<T, T> m_maxRange;
    value_type m_value;
};

}  // namespace engine

