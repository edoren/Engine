#pragma once

#include <Util/Prerequisites.hpp>

#include <string>

namespace engine::utf {

////////////////////////////////////////////////////////////////////////////////
// Internal UTF functions
////////////////////////////////////////////////////////////////////////////////
namespace internal {

template <typename Iter>
constexpr char32 GetCodePoint8(const Iter begin, const Iter end) {
    static_assert(type::is_forward_iterator_v<Iter>, "Value should be a forward iterator");
    static_assert(std::is_integral_v<type::iterator_underlying_type_t<Iter>>,
                  "Iterator internal type should be an integer");
    static_assert(sizeof(type::iterator_underlying_type_t<Iter>) == sizeof(char8),
                  "Iterator internal type has an invalid size");

    ENGINE_ASSERT((end - begin) != 0, "UTF-8 should have minimum one code unit");
    ENGINE_ASSERT((end - begin) <= 4, "UTF-8 should have maximum four code units");

    char32 unicodeCodePoint = 0;

    // TODO: Repeated code with utf::CodeUnit::getSize
    size_t size = 1;
    if ((*begin & 0x80) == 0x00) {
        size = 1;
    } else if ((*begin & 0xE0) == 0xC0) {
        size = 2;
    } else if ((*begin & 0xF0) == 0xE0) {
        size = 3;
    } else {
        ENGINE_ASSERT((*begin & 0xF8) == 0xF0, "Error getting UTF-8 size");
        size = 4;
    }

    switch (size) {
        case 4:
            unicodeCodePoint = ((*begin & 0x7) << 18) | ((*(begin + 1) & 0x3F) << 12) | ((*(begin + 2) & 0x3F) << 6) |
                               (*(begin + 3) & 0x3F);
            break;
        case 3:
            unicodeCodePoint = ((*begin & 0xF) << 12) | ((*(begin + 1) & 0x3F) << 6) | (*(begin + 2) & 0x3F);
            break;
        case 2:
            unicodeCodePoint = ((*begin & 0x1F) << 6) | (*(begin + 1) & 0x3F);
            break;
        case 1:
            unicodeCodePoint = (*begin & 0x7F);
            break;
        default:
            // Should not happen
            break;
    }

    return unicodeCodePoint;
}

template <typename Iter>
constexpr char32 GetCodePoint16(const Iter begin, const Iter end) {
    static_assert(type::is_forward_iterator_v<Iter>, "Value should be a forward iterator");
    static_assert(std::is_integral_v<type::iterator_underlying_type_t<Iter>>,
                  "Iterator internal type should be an integer");
    static_assert(sizeof(type::iterator_underlying_type_t<Iter>) == sizeof(char16),
                  "Iterator internal type has an invalid size");

    ENGINE_ASSERT((end - begin) != 0, "UTF-16 should have minimum one code unit");
    ENGINE_ASSERT((end - begin) <= 2, "UTF-16 should have maximum two code units");

    // TODO: Repeated code with utf::CodeUnit::getSize
    size_t size = 1;
    if ((*begin & 0xFC00) == 0xD800) {
        size = 2;
    }

    char32 unicodeCodePoint = 0;

    switch (size) {
        case 2:
            unicodeCodePoint = ((char32(*begin & 0x3FF) << 10) | char32(*(begin + 1) & 0x3FF)) + 0x10000;
            break;
        case 1:
            unicodeCodePoint = char32(*begin);
            break;
        default:
            break;
    }

    return unicodeCodePoint;
}

template <typename Iter>
constexpr char32 GetCodePoint32(const Iter begin, const Iter end) {
    static_assert(type::is_forward_iterator_v<Iter>, "Value should be a forward iterator");
    static_assert(std::is_integral<type::iterator_underlying_type_t<Iter>>::value,
                  "Iterator internal type should be an integer");
    static_assert(sizeof(type::iterator_underlying_type_t<Iter>) == sizeof(char32),
                  "Iterator internal type has an invalid size");

    ENGINE_ASSERT((end - begin) == 1, "UTF-32 should have minimum one code unit");

    return *begin & 0x1FFFFF;
}

template <typename Iter>
constexpr Iter Next8(Iter begin, Iter end) {
    static_assert(type::is_forward_iterator_v<Iter>, "Value should be a forward iterator");
    static_assert(std::is_integral<type::iterator_underlying_type_t<Iter>>::value,
                  "Iterator internal type should be an integer");
    static_assert(sizeof(type::iterator_underlying_type_t<Iter>) == sizeof(char8),
                  "Iterator internal type has an invalid size");

    Iter s = begin;

    if ((*s & 0xF8) == 0xF0) {
        // Verify out of range
        if ((s + 4) > end) {
            return s;
        }

        // Verify invalid range from 0xF5 to 0xF7
        if (*s >= 0xF5 && *s <= 0xF7) {
            return s;
        }

        // Ensure that the top 5 bits of this 4-byte UTF-8
        // codepoint were not 0, as then we could have used
        // one of the smaller encodings
        if ((*s & 0x07) == 0 && (*(s + 1) & 0x30) == 0) {
            return s;
        }

        // Ensure each of the 3 following bytes in this 4-byte
        // UTF-8 codepoint began with 0b10xxxxx
        for (int32 i = 1; i < 3; i++) {
            if ((*(s + i) & 0xC0) != 0x80) {
                return s;
            }
        }

        // 4-byte UTF-8 code point (began with 0b11110xxx)
        return s + 4;
    }

    if ((*s & 0xF0) == 0xE0) {
        // Verify out of range
        if ((s + 3) > end) {
            return s;
        }

        // Ensure that the top 5 bits of this 3-byte UTF-8
        // codepoint were not 0, as then we could have used
        // one of the smaller encodings
        if ((*s & 0x0f) == 0 && (*(s + 1) & 0x20) == 0) {
            return s;
        }

        // Ensure each of the 2 following bytes in this 3-byte
        // UTF-8 codepoint began with 0b10xxxxxx
        for (int32 i = 1; i < 2; i++) {
            if ((*(s + i) & 0xC0) != 0x80) {
                return s;
            }
        }

        // 3-byte UTF-8 code point (began with 0b1110xxxx)
        return s + 3;
    }

    if ((*s & 0xE0) == 0xC0) {
        // Verify out of range
        if ((s + 2) > end) {
            return s;
        }

        // Ensure that the top 4 bits of this 2-byte UTF-8
        // codepoint were not 0, as then we could have used
        // one of the smaller encodings
        if ((*s & 0x1e) == 0) {
            return s;
        }

        // Verify invalid 0xC0 and 0xC1 codepoints
        if (*s == 0xC0 || *s == 0xC1) {
            return s;
        }

        // Ensure the 1 following byte in this 2-byte
        // UTF-8 codepoint began with 0b10xxxxxx
        if ((*(s + 1) & 0xC0) != 0x80) {
            return s;
        }

        // 2-byte UTF-8 code point (began with 0b110xxxxx)
        return s + 2;
    }

    // 1-byte ascii (began with 0b0xxxxxxx)
    if ((*s & 0x80) == 0x00) {
        if ((s + 1) > end) {  // Verify out of range
            return s;
        }
        return s + 1;
    }

    // We have an invalid 0b1xxxxxxx UTF-8 code point entry
    return s;
}

template <typename Iter>
constexpr Iter Next16(Iter begin, Iter end) {
    static_assert(type::is_forward_iterator_v<Iter>, "Value should be a forward iterator");
    static_assert(std::is_integral<type::iterator_underlying_type_t<Iter>>::value,
                  "Iterator internal type should be an integer");
    static_assert(sizeof(type::iterator_underlying_type_t<Iter>) == sizeof(char16),
                  "Iterator internal type has an invalid size");

    Iter s = begin;

    // Check the range is between 0x0000 to 0xD7FF and 0xE000 to 0xFFFF
    if ((*s >= 0x0000 && *s <= 0xD7FF) || (*s >= 0xE000 && *s <= 0xFFFF)) {
        if ((s + 1) > end) {  // Verify out of range
            return s;
        }
        return s + 1;
    }

    // Check the first 16 bit code unit to be 0b110110yyyyyyyyyy
    // Check the second 16 bit code unit to be 0b110111xxxxxxxxxx
    if ((*s & 0xFC00) == 0xD800 && (*(s + 1) & 0xFC00) == 0xDC00) {
        if ((s + 2) > end) {  // Verify out of range
            return s;
        }
        return s + 2;
    }

    // We have an invalid UTF-16 code point entry
    return s;
}

template <typename Iter>
constexpr Iter Next32(Iter begin, Iter end) {
    static_assert(type::is_forward_iterator_v<Iter>, "Value should be a forward iterator");
    static_assert(std::is_integral<type::iterator_underlying_type_t<Iter>>::value,
                  "Iterator internal type should be an integer");
    static_assert(sizeof(type::iterator_underlying_type_t<Iter>) == sizeof(char32),
                  "Iterator internal type has an invalid size");

    Iter s = begin;

    // Check that the first 11 bits are zero. 0b00000000000xxxxxxxxxxxxxxxxxxxxx
    if ((*s & 0xFFE00000) != 0) {
        return s;
    }

    // Verify out of range
    if ((s + 1) > end) {
        return s;
    }

    return s + 1;
}

template <typename Iter>
constexpr Iter Prior8(Iter end, Iter begin) {
    static_assert(type::is_bidirectional_iterator_v<Iter>, "Value should be a bidirectional iterator");
    static_assert(std::is_integral<type::iterator_underlying_type_t<Iter>>::value,
                  "Iterator internal type should be an integer");
    static_assert(sizeof(type::iterator_underlying_type_t<Iter>) == sizeof(char8),
                  "Iterator internal type has an invalid size");

    ENGINE_ASSERT((end - begin) >= 0, "The end iterator should be same or higher that the begin iterator");

    size_t bytesToCheck = (end - begin) >= 4 ? 4 : (end - begin);

    for (size_t i = 1; i < bytesToCheck + 1; i++) {
        auto it = end - i;
        if ((i == 1 && (*it & 0x80) == 0x00 && IsValid<UTF_8>(it, end)) ||
            (i == 2 && (*it & 0xE0) == 0xC0 && IsValid<UTF_8>(it, end)) ||
            (i == 3 && (*it & 0xF0) == 0xE0 && IsValid<UTF_8>(it, end)) ||
            (i == 4 && (*it & 0xF8) == 0xF0 && IsValid<UTF_8>(it, end))) {
            return it;
        }
    }

    return end;
}

template <typename Iter>
constexpr Iter Prior16(Iter end, Iter begin) {
    static_assert(type::is_bidirectional_iterator_v<Iter>, "Value should be a bidirectional iterator");
    static_assert(std::is_integral<type::iterator_underlying_type_t<Iter>>::value,
                  "Iterator internal type should be an integer");
    static_assert(sizeof(type::iterator_underlying_type_t<Iter>) == sizeof(char16),
                  "Iterator internal type has an invalid size");

    ENGINE_ASSERT((end - begin) >= 0, "The end iterator should be same or higher that the begin iterator");

    auto it = end - 1;
    if ((*it >= 0x0000 && *it <= 0xD7FF) || (*it >= 0xE000 && *it <= 0xFFFF)) {
        return it;
    }
    if (((*it & 0xFC00) == 0xDC00) || ((*(it - 1) & 0xFC00) == 0xD800)) {
        return it - 1;
    }

    return end;
}

template <typename Iter>
constexpr Iter Prior32(Iter end, Iter begin) {
    static_assert(type::is_bidirectional_iterator_v<Iter>, "Value should be a bidirectional iterator");
    static_assert(std::is_integral<type::iterator_underlying_type_t<Iter>>::value,
                  "Iterator internal type should be an integer");
    static_assert(sizeof(type::iterator_underlying_type_t<Iter>) == sizeof(char32),
                  "Iterator internal type has an invalid size");

    ENGINE_ASSERT((end - begin) >= 0, "The end iterator should be same or higher that the begin iterator");

    return end - 1;
}

}  // namespace internal

////////////////////////////////////////////////////////////////////////////////
// CodeUnit
////////////////////////////////////////////////////////////////////////////////

template <Encoding Base>
constexpr CodeUnit<Base>::CodeUnit(data_type data) : m_unit(std::move(data)) {}

template <Encoding Base>
constexpr CodeUnit<Base>::CodeUnit(char32 codePoint) : m_unit() {
    if constexpr (Base == UTF_8) {
        if (codePoint >= 0x0000 && codePoint <= 0x007F) {
            m_unit[0] = static_cast<uint8>(codePoint);
        } else if (codePoint >= 0x0080 && codePoint <= 0x07FF) {
            m_unit[0] = static_cast<uint8>(((codePoint & 0x7C0) >> 6) | 0xC0);
            m_unit[1] = static_cast<uint8>((codePoint & 0x3F) | 0x80);
        } else if (codePoint >= 0x0800 && codePoint <= 0xFFFF) {
            m_unit[0] = static_cast<uint8>(((codePoint & 0xF000) >> 12) | 0xE0);
            m_unit[1] = static_cast<uint8>(((codePoint & 0xFC0) >> 6) | 0x80);
            m_unit[2] = static_cast<uint8>((codePoint & 0x3F) | 0x80);
        } else if (codePoint >= 0x10000 && codePoint <= 0x10FFFF) {
            m_unit[0] = static_cast<uint8>(((codePoint & 0x1C0000) >> 18) | 0xF0);
            m_unit[1] = static_cast<uint8>(((codePoint & 0x3F000) >> 12) | 0x80);
            m_unit[2] = static_cast<uint8>(((codePoint & 0xFC0) >> 6) | 0x80);
            m_unit[3] = static_cast<uint8>((codePoint & 0x3F) | 0x80);
        }
    } else if constexpr (Base == UTF_16) {
        if ((codePoint >= 0x0000 && codePoint <= 0xD7FF) || (codePoint >= 0xE000 && codePoint <= 0xFFFF)) {
            m_unit[0] = static_cast<uint16>(codePoint);
        } else if (codePoint >= 0x010000 && codePoint <= 0x10FFFF) {
            char32 u = codePoint - 0x10000;
            m_unit[0] = static_cast<uint16>(((u & 0xFFC00) | 0x3600000) >> 10);
            m_unit[1] = static_cast<uint16>(((u & 0x3FF) | 0xDC00));
        }
    } else if constexpr (Base == UTF_32) {
        m_unit[0] = codePoint;
    }
}

template <Encoding Base>
template <typename Iter>
constexpr CodeUnit<Base>::CodeUnit(const Iter begin, const Iter end) : CodeUnit(begin, (end - begin)) {}

template <Encoding Base>
template <typename Iter>
constexpr CodeUnit<Base>::CodeUnit(const Iter begin, const size_type size) {
    static_assert(type::is_forward_iterator_v<Iter>, "Value should be a forward iterator");
    static_assert(std::is_integral<type::iterator_underlying_type_t<Iter>>::value,
                  "Iterator internal type should be an integer");
    static_assert(sizeof(type::iterator_underlying_type_t<Iter>) == GetEncodingSize(Base),
                  "Iterator internal type has an invalid size");

    const auto* pBegin = reinterpret_cast<const value_type*>(&(*begin));

    ENGINE_ASSERT(IsValid<Base>(pBegin, pBegin + size), "Invalid UTF code unit");

    if constexpr (Base == UTF_8) {
        ENGINE_ASSERT(size <= 4, "UTF-8 should have at least four 8 bit values");
    } else if constexpr (Base == UTF_16) {
        ENGINE_ASSERT(size <= 2, "UTF-16 should have at least two 16 bit values");
    } else if constexpr (Base == UTF_32) {
        ENGINE_ASSERT(size <= 1, "UTF-32 should have at least one 32 bit value");
    }

    m_unit.fill(0x0);
    if (size != 0) {
        std::memcpy(m_unit.data(), pBegin, sizeof(CodeUnit::value_type) * size);
    }
}

template <Encoding Base>
constexpr char32 CodeUnit<Base>::getCodePoint() const {
    const auto* begin = m_unit.data();
    const auto* end = m_unit.data() + getSize();
    return GetCodePoint<Base>(begin, end);
}

template <Encoding Base>
constexpr const typename CodeUnit<Base>::value_type* CodeUnit<Base>::begin() const {
    return m_unit.data();
}

template <Encoding Base>
constexpr const typename CodeUnit<Base>::value_type* CodeUnit<Base>::end() const {
    return begin() + getSize();
}

template <Encoding Base>
constexpr const typename CodeUnit<Base>::data_type& CodeUnit<Base>::getData() const {
    return m_unit;
}

template <Encoding Base>
constexpr typename CodeUnit<Base>::size_type CodeUnit<Base>::getSize() const {
    if constexpr (Base == UTF_8) {
        if ((m_unit[0] & 0x80) == 0x00) {
            return 1;
        }
        if ((m_unit[0] & 0xE0) == 0xC0) {
            return 2;
        }
        if ((m_unit[0] & 0xF0) == 0xE0) {
            return 3;
        }
        ENGINE_ASSERT((m_unit[0] & 0xF8) == 0xF0, "Error getting UTF-8 size");
        return 4;
    } else if constexpr (Base == UTF_16) {
        if ((m_unit[0] & 0xFC00) == 0xD800) {
            return 2;
        }
        return 1;
    } else if constexpr (Base == UTF_32) {
        return m_unit.size();
    }
}

template <Encoding Base>
constexpr bool CodeUnit<Base>::operator==(const CodeUnit& right) const {
    return operator==(right.m_unit);
}

template <Encoding Base>
constexpr bool CodeUnit<Base>::operator==(const data_type& right) const {
    return std::memcmp(m_unit.data(), right.data(), 4) == 0;
}

template <Encoding Base>
constexpr bool CodeUnit<Base>::operator==(char8 right) const {
    return m_unit[0] == right;
}

////////////////////////////////////////////////////////////////////////////////
// CodeUnitRange
////////////////////////////////////////////////////////////////////////////////

template <Encoding Base, typename Iter>
constexpr CodeUnitRange<Base, Iter>::CodeUnitRange() : m_range(nullptr, nullptr) {}

template <Encoding Base, typename Iter>
constexpr CodeUnitRange<Base, Iter>::CodeUnitRange(std::pair<pointed_type, pointed_type>&& range)
      : m_range(std::move(range)) {}

template <Encoding Base, typename Iter>
constexpr CodeUnitRange<Base, Iter>::CodeUnitRange(const Iter begin, const Iter end) : m_range(&(*begin), &(*end)) {}

template <Encoding Base, typename Iter>
constexpr CodeUnit<Base> CodeUnitRange<Base, Iter>::get() const {
    return CodeUnit<Base>(m_range.first, m_range.second);
}

template <Encoding BaseFrom, typename Iter>
template <Encoding BaseTo>
constexpr CodeUnit<BaseTo> CodeUnitRange<BaseFrom, Iter>::getAs() const {
    if constexpr (BaseFrom == BaseTo) {
        return CodeUnit<BaseFrom>(m_range.first, m_range.second);
    } else {
        return CodeUnit<BaseTo>(GetCodePoint<BaseFrom>(m_range.first, m_range.second));
    }
}

template <Encoding Base, typename Iter>
constexpr typename CodeUnitRange<Base, Iter>::pointed_type CodeUnitRange<Base, Iter>::begin() const {
    return m_range.first;
}

template <Encoding Base, typename Iter>
constexpr typename CodeUnitRange<Base, Iter>::pointed_type CodeUnitRange<Base, Iter>::end() const {
    return m_range.second;
}

template <Encoding Base, typename Iter>
constexpr auto CodeUnitRange<Base, Iter>::getRange() const -> const std::pair<pointed_type, pointed_type>& {
    return m_range;
}

template <Encoding Base, typename Iter>
template <typename Iter2>
constexpr bool CodeUnitRange<Base, Iter>::operator==(const CodeUnitRange<Base, Iter2>& other) const {
    size_type size = (m_range.second - m_range.first);
    size_type otherSize = (other.m_range.second - other.m_range.first);
    return otherSize == size && memcmp(m_range.first, other.m_range.first, size) == 0;
}

template <Encoding Base, typename Iter>
template <typename Iter2>
constexpr bool CodeUnitRange<Base, Iter>::operator!=(const CodeUnitRange<Base, Iter2>& other) const {
    return !(*this == other);
}

////////////////////////////////////////////////////////////////////////////////
// Iterator
////////////////////////////////////////////////////////////////////////////////

template <Encoding Base, typename T>
constexpr Iterator<Base, T>::Iterator(std::pair<pointed_type, pointed_type> maxRange, pointed_type begin)
      : m_maxRange(std::move(maxRange)) {
    // Post end iterator
    if (begin >= maxRange.second) {
        m_ref = std::make_pair(maxRange.second, maxRange.second + 1);
        return;
    }

    // Prior begin iterator
    if (begin < maxRange.first) {  // End of iterator
        m_ref = std::make_pair(maxRange.first - 1, maxRange.first);
        return;
    }

    const auto* end = begin;
    if constexpr (Base == UTF_8) {
        if ((*begin & 0x80) == 0x00) {
            end += 1;
        } else if ((*begin & 0xE0) == 0xC0) {
            end += 2;
        } else if ((*begin & 0xF0) == 0xE0) {
            end += 3;
        } else if ((*begin & 0xF8) == 0xF0) {
            end += 4;
        }
    } else if constexpr (Base == UTF_16) {
        if ((*begin >= 0x0000 && *begin <= 0xD7FF) || (*begin >= 0xE000 && *begin <= 0xFFFF)) {
            end += 1;
        } else if (*begin >= 0x010000 && *begin <= 0x10FFFF) {
            end += 2;
        }
    } else if constexpr (Base == UTF_32) {
        end += 1;
    }

    m_ref = std::make_pair(begin, end);
}

template <Encoding Base, typename T>
constexpr typename Iterator<Base, T>::reference Iterator<Base, T>::operator*() {
    return m_ref;
}

template <Encoding Base, typename T>
constexpr typename Iterator<Base, T>::pointer Iterator<Base, T>::operator->() {
    return &m_ref;
}

template <Encoding Base, typename T>
constexpr Iterator<Base, T> Iterator<Base, T>::operator+(size_type num) {
    // TODO: Improve this iteration for edge cases
    // Check if already on the end
    const auto* newBegin = m_ref.begin();
    for (size_type i = 0; i < num; i++) {
        const auto* it = Next<Base>(newBegin, m_maxRange.second);
        if (it == newBegin) {
            // Error getting next
            return Iterator(m_maxRange, m_maxRange.second + 1);
        }
        newBegin = it;
    }
    return Iterator(m_maxRange, newBegin);
}

template <Encoding Base, typename T>
constexpr Iterator<Base, T>& Iterator<Base, T>::operator+=(size_type num) {
    *this = *this + num;
    return *this;
}

template <Encoding Base, typename T>
constexpr Iterator<Base, T>& Iterator<Base, T>::operator++() {
    return *this += 1;
}

template <Encoding Base, typename T>
constexpr Iterator<Base, T> Iterator<Base, T>::operator++(int) {
    Iterator temp(*this);
    *this += 1;
    return temp;
}

template <Encoding Base, typename T>
constexpr Iterator<Base, T> Iterator<Base, T>::operator-(size_type num) {
    // TODO: Improve this iteration for edge cases
    // Check if already on the end
    const auto* newBegin = m_ref.begin();
    for (size_type i = 0; i < num; i++) {
        const auto* it = Prior<Base>(newBegin, m_maxRange.first);
        if (it == newBegin) {
            // Error getting next
            return Iterator(m_maxRange, m_maxRange.first);
        }
        newBegin = it;
    }
    return Iterator(m_maxRange, newBegin);
}

template <Encoding Base, typename T>
constexpr Iterator<Base, T>& Iterator<Base, T>::operator-=(size_type num) {
    *this = *this - num;
    return *this;
}

template <Encoding Base, typename T>
constexpr Iterator<Base, T>& Iterator<Base, T>::operator--() {
    return *this -= 1;
}

template <Encoding Base, typename T>
constexpr Iterator<Base, T> Iterator<Base, T>::operator--(int) {
    Iterator temp(*this);
    *this -= 1;
    return temp;
}

template <Encoding Base, typename T>
constexpr bool Iterator<Base, T>::operator<(const Iterator& other) const {
    return m_ref.getRange() < other.m_ref.getRange();
}

template <Encoding Base, typename T>
constexpr bool Iterator<Base, T>::operator>(const Iterator& other) const {
    return other < *this;
}

template <Encoding Base, typename T>
constexpr bool Iterator<Base, T>::operator<=(const Iterator& other) const {
    return !(other < *this);
}

template <Encoding Base, typename T>
constexpr bool Iterator<Base, T>::operator>=(const Iterator& other) const {
    return !(*this < other);
}

template <Encoding Base, typename T>
constexpr bool Iterator<Base, T>::operator==(const Iterator& other) const {
    return m_ref.getRange() == other.m_ref.getRange() && m_maxRange == other.m_maxRange;
}

template <Encoding Base, typename T>
constexpr bool Iterator<Base, T>::operator!=(const Iterator& other) const {
    return !(*this == other);
}

template <Encoding Base, typename T>
constexpr typename Iterator<Base, T>::pointed_type Iterator<Base, T>::getPtr() const {
    return m_ref.getRange().first;
}

////////////////////////////////////////////////////////////////////////////////
// Static Functions
////////////////////////////////////////////////////////////////////////////////

constexpr size_t GetEncodingSize(Encoding encoding) {
    switch (encoding) {
        case Encoding::UTF_8:
            return 1;
        case Encoding::UTF_16:
            return 2;
        case Encoding::UTF_32:
            return 4;
    }
}

template <Encoding BaseFrom, Encoding BaseTo, typename T, typename Ret, typename>
constexpr T UtfToUtf(T begin, T end, std::basic_string<Ret>* result) {
    size_t sizeBytes = 0;

    auto it = ForEach<BaseFrom>(begin, end, [&sizeBytes](auto&& codeUnitRange) {
        auto convertedCodeUnit = codeUnitRange.template getAs<BaseTo>();
        sizeBytes += convertedCodeUnit.getSize();
    });

    if (it != end) {
        return it;
    }

    result->reserve(result->size() + sizeBytes);

    if constexpr (BaseFrom == BaseTo) {
        it = ForEach<BaseFrom>(begin, end, [&result](auto&& codeUnitRange) {
            result->insert(result->end(), codeUnitRange.begin(), codeUnitRange.end());
        });
    } else {
        it = ForEach<BaseFrom>(begin, end, [&result](auto&& codeUnitRange) {
            auto convertedCodeUnit = codeUnitRange.template getAs<BaseTo>();
            for (auto val : convertedCodeUnit) {
                *result += val;
            }
        });
    }

    return it;
}

template <Encoding Base, typename Iter>
constexpr char32 GetCodePoint(const Iter begin, const Iter end) {
    if constexpr (Base == UTF_8) {
        return internal::GetCodePoint8(begin, end);
    } else if constexpr (Base == UTF_16) {
        return internal::GetCodePoint16(begin, end);
    } else if constexpr (Base == UTF_32) {
        return internal::GetCodePoint32(begin, end);
    }
}

template <Encoding Base, typename Iter>
constexpr Iter Next(Iter begin, Iter end) {
    if (begin >= end) {
        return end;
    }

    if constexpr (Base == UTF_8) {
        return internal::Next8(begin, end);
    } else if constexpr (Base == UTF_16) {
        return internal::Next16(begin, end);
    } else if constexpr (Base == UTF_32) {
        return internal::Next32(begin, end);
    }
}

template <Encoding Base, typename Iter>
constexpr Iter Prior(Iter end, Iter begin) {
    if (end <= begin) {
        return begin;
    }

    if constexpr (Base == UTF_8) {
        return internal::Prior8(end, begin);
    } else if constexpr (Base == UTF_16) {
        return internal::Prior16(end, begin);
    } else if constexpr (Base == UTF_32) {
        return internal::Prior32(end, begin);
    }
}

template <Encoding Base, typename Iter, typename Func>
constexpr Iter ForEach(Iter begin, Iter end, Func fn) {
    Iter s = begin;

    while (s < end) {
        auto next = Next<Base>(s, end);
        if (next == s) {
            // Error encoding
            return next;
        }
        fn(CodeUnitRange<Base, decltype(s)>(s, next));
        s = next;
    }

    return end;
}

template <Encoding Base, typename Iter>
constexpr size_t GetSize(const Iter begin, const Iter end) {
    size_t size = 0;
    auto it = ForEach<Base>(begin, end, [&size](auto /*unused*/) { size++; });
    if (it != end) {
        size = size_t(-1);
    }
    return size;
}

template <Encoding Base, typename Iter>
constexpr bool IsValid(const Iter begin, const Iter end) {
    return ForEach<Base>(begin, end, [](auto /*unused*/) {}) == end;
}

}  // namespace engine::utf

// namespace std {

// template <typename Func, Encoding Base, typename T>
// constexpr decltype(auto) apply(Func&& func, engine::utf::CodeUnit<Base> t) {
//     auto [begin, end] = t;
//     return std::invoke(std::move(func), begin, end);
// }

// }  // namespace std
