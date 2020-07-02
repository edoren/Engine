#pragma once

#include <Util/Prerequisites.hpp>

#include <Util/UTF.hpp>

#include <utility>

namespace engine {

template <size_t Base, typename T>
constexpr UTFIterator<Base, T>::ValueRef::ValueRef(std::pair<T, T>&& value) : m_value(value) {}

template <size_t Base, typename T>
constexpr utf::CodeUnit<Base> UTFIterator<Base, T>::ValueRef::getUnit() const {
    return utf::CodeUnit<Base>(m_value.first, m_value.second);
}

template <size_t Base, typename T>
constexpr typename UTFIterator<Base, T>::reference UTFIterator<Base, T>::operator*() {
    return m_ref;
}

template <size_t Base, typename T>
constexpr typename UTFIterator<Base, T>::pointer UTFIterator<Base, T>::operator->() {
    return &m_ref;
}

template <size_t Base, typename T>
constexpr UTFIterator<Base, T> UTFIterator<Base, T>::operator+(uint32 num) {
    // TODO: Improve this iteration for edge cases
    // Check if already on the end
    auto newBegin = m_ref.m_value.first;
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

template <size_t Base, typename T>
constexpr UTFIterator<Base, T>& UTFIterator<Base, T>::operator+=(uint32 num) {
    *this = *this + num;
    return *this;
}

template <size_t Base, typename T>
constexpr UTFIterator<Base, T>& UTFIterator<Base, T>::operator++() {
    return *this += 1;
}

template <size_t Base, typename T>
constexpr UTFIterator<Base, T> UTFIterator<Base, T>::operator++(int) {
    UTFIterator temp(*this);
    *this += 1;
    return temp;
}

template <size_t Base, typename T>
constexpr bool UTFIterator<Base, T>::operator==(const UTFIterator& other) const {
    return m_ref.m_value == other.m_ref.m_value && m_maxRange == other.m_maxRange;
}

template <size_t Base, typename T>
constexpr bool UTFIterator<Base, T>::operator!=(const UTFIterator& other) const {
    return !(*this == other);
}

template <size_t Base, typename T>
constexpr void UTFIterator<Base, T>::swap(UTFIterator& other) {
    std::swap(m_maxRange, other.m_maxRange);
    std::swap(m_ref.m_value, other.m_ref.m_value);
}

template <size_t Base, typename T>
constexpr T UTFIterator<Base, T>::getPtr() const {
    return m_ref.m_value.first;
}

template <size_t Base, typename T>
constexpr UTFIterator<Base, T>::UTFIterator(std::pair<T, T> maxRange, T begin)
      : m_maxRange(std::move(maxRange)),
        m_ref(std::make_pair(begin, begin)) {
    if (maxRange.second == begin) {  // End of iterator
        return;
    }

    if constexpr (Base == 8) {
        if ((*begin & 0x80) == 0x00) {
            m_ref.m_value.second += 1;
        } else if ((*begin & 0xE0) == 0xC0) {
            m_ref.m_value.second += 2;
        } else if ((*begin & 0xF0) == 0xE0) {
            m_ref.m_value.second += 3;
        } else if ((*begin & 0xF8) == 0xF0) {
            m_ref.m_value.second += 4;
        }
    } else if constexpr (Base == 16) {
        if ((*begin >= 0x0000 && *begin <= 0xD7FF) || (*begin >= 0xE000 && *begin <= 0xFFFF)) {
            m_ref.m_value.second += 1;
        } else if (*begin >= 0x010000 && *begin <= 0x10FFFF) {
            m_ref.m_value.second += 2;
        }
    } else if constexpr (Base == 32) {
        m_ref.m_value.second += 1;
    }
}

}  // namespace engine
