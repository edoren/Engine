#pragma once

#include <Util/Prerequisites.hpp>

#include <string>

#include <cassert>

namespace engine {

namespace utf {

namespace internal {

template <typename T>
constexpr char32 CodePointFromUTF8(T begin, T end) {
    static_assert(type::is_forward_iterator<T>::value, "Value should be a forward iterator");
    static_assert(std::is_integral<type::iterator_underlying_type_t<T>>::value,
                  "Iterator internal type should be an integer");
    static_assert(sizeof(type::iterator_underlying_type_t<T>) == sizeof(char8),
                  "Iterator internal type has an invalid size");

    ENGINE_ASSERT((end - begin) != 0, "UTF-8 should have minimum one code unit");
    ENGINE_ASSERT((end - begin) <= 4, "UTF-8 should have maximum four code units");

    char32 unicodeCodePoint = 0;

    switch (end - begin) {
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

template <typename T>
constexpr char32 CodePointFromUTF16(T begin, T end) {
    static_assert(type::is_forward_iterator<T>::value, "Value should be a forward iterator");
    static_assert(std::is_integral<type::iterator_underlying_type_t<T>>::value,
                  "Iterator internal type should be an integer");
    static_assert(sizeof(type::iterator_underlying_type_t<T>) == sizeof(char16),
                  "Iterator internal type has an invalid size");

    ENGINE_ASSERT((end - begin) != 0, "UTF-16 should have minimum one code unit");
    ENGINE_ASSERT((end - begin) <= 2, "UTF-16 should have maximum two code units");

    char32 unicodeCodePoint = 0;

    switch (end - begin) {
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

template <typename T>
constexpr char32 CodePointFromUTF32(T begin, T end) {
    static_assert(type::is_forward_iterator<T>::value, "Value should be a forward iterator");
    static_assert(std::is_integral<type::iterator_underlying_type_t<T>>::value,
                  "Iterator internal type should be an integer");
    static_assert(sizeof(type::iterator_underlying_type_t<T>) == sizeof(char32),
                  "Iterator internal type has an invalid size");

    ENGINE_ASSERT((end - begin) == 1, "UTF-32 should have minimum one code unit");

    return *begin & 0x1FFFFF;
}

template <typename T>
constexpr CodeUnit<UTF_8> CodePointToUTF8(T codePoint) {
    static_assert(std::is_integral<T>::value, "Code point should be an integer");
    static_assert(sizeof(T) == sizeof(char32), "Code point has an invalid size");

    std::array<uint8, 4> codeUnit;

    size_t size = 0;
    if (codePoint >= 0x0000 && codePoint <= 0x007F) {
        codeUnit[0] = static_cast<uint8>(codePoint);
        size = 1;
    } else if (codePoint >= 0x0080 && codePoint <= 0x07FF) {
        codeUnit[0] = ((codePoint & 0x7C0) >> 6) | 0xC0;
        codeUnit[1] = (codePoint & 0x3F) | 0x80;
        size = 2;
    } else if (codePoint >= 0x0800 && codePoint <= 0xFFFF) {
        codeUnit[0] = ((codePoint & 0xF000) >> 12) | 0xE0;
        codeUnit[1] = ((codePoint & 0xFC0) >> 6) | 0x80;
        codeUnit[2] = (codePoint & 0x3F) | 0x80;
        size = 3;
    } else if (codePoint >= 0x10000 && codePoint <= 0x10FFFF) {
        codeUnit[0] = ((codePoint & 0x1C0000) >> 18) | 0xF0;
        codeUnit[1] = ((codePoint & 0x3F000) >> 12) | 0x80;
        codeUnit[2] = ((codePoint & 0xFC0) >> 6) | 0x80;
        codeUnit[3] = (codePoint & 0x3F) | 0x80;
        size = 4;
    }

    return CodeUnit<UTF_8>(codeUnit.data(), size);
}

template <typename T>
constexpr CodeUnit<UTF_16> CodePointToUTF16(T codePoint) {
    static_assert(std::is_integral<T>::value, "Code point should be an integer");
    static_assert(sizeof(T) == sizeof(char32), "Code point has an invalid size");

    std::array<uint16, 2> codeUnit;

    size_t size = 0;
    if ((codePoint >= 0x0000 && codePoint <= 0xD7FF) || (codePoint >= 0xE000 && codePoint <= 0xFFFF)) {
        codeUnit[0] = static_cast<uint16>(codePoint);
        size = 1;
    } else if (codePoint >= 0x010000 && codePoint <= 0x10FFFF) {
        char32 u = codePoint - 0x10000;
        codeUnit[0] = ((u & 0xFFC00) | 0x3600000) >> 10;
        codeUnit[1] = ((u & 0x3FF) | 0xDC00);
        size = 2;
    }

    return CodeUnit<UTF_16>(codeUnit.data(), size);
}

template <typename T>
constexpr CodeUnit<UTF_32> CodePointToUTF32(T codePoint) {
    static_assert(std::is_integral<T>::value, "Code point should be an integer");
    static_assert(sizeof(T) == sizeof(char32), "Code point has an invalid size");

    return CodeUnit<UTF_32>(&codePoint, 1);
}

template <typename T>
constexpr T NextUTF8(T begin, T end) {
    static_assert(type::is_forward_iterator<T>::value, "Value should be a forward iterator");
    static_assert(std::is_integral<type::iterator_underlying_type_t<T>>::value,
                  "Iterator internal type should be an integer");
    static_assert(sizeof(type::iterator_underlying_type_t<T>) == sizeof(char8),
                  "Iterator internal type has an invalid size");

    T s = begin;

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

template <typename T>
constexpr T NextUTF16(T begin, T end) {
    static_assert(type::is_forward_iterator<T>::value, "Value should be a forward iterator");
    static_assert(std::is_integral<type::iterator_underlying_type_t<T>>::value,
                  "Iterator internal type should be an integer");
    static_assert(sizeof(type::iterator_underlying_type_t<T>) == sizeof(char16),
                  "Iterator internal type has an invalid size");

    T s = begin;

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

template <typename T>
constexpr T NextUTF32(T begin, T end) {
    static_assert(type::is_forward_iterator<T>::value, "Value should be a forward iterator");
    static_assert(std::is_integral<type::iterator_underlying_type_t<T>>::value,
                  "Iterator internal type should be an integer");
    static_assert(sizeof(type::iterator_underlying_type_t<T>) == sizeof(char32),
                  "Iterator internal type has an invalid size");

    T s = begin;

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

template <typename T>
constexpr T PriorUTF8(T end, T begin) {
    static_assert(type::is_bidirectional_iterator<T>::value, "Value should be a bidirectional iterator");
    static_assert(std::is_integral<type::iterator_underlying_type_t<T>>::value,
                  "Iterator internal type should be an integer");
    static_assert(sizeof(type::iterator_underlying_type_t<T>) == sizeof(char8),
                  "Iterator internal type has an invalid size");

    ENGINE_ASSERT((end - begin) >= 0, "The end iterator should be same or higher that the begin iterator");

    size_t bytesToCheck = (end - begin) >= 4 ? 4 : (end - begin);

    for (size_t i = 1; i < bytesToCheck + 1; i++) {
        auto it = end - i;
        if ((i == 1 && (*it & 0x80) == 0x00 && IsValidUTF<UTF_8>(it, end)) ||
            (i == 2 && (*it & 0xE0) == 0xC0 && IsValidUTF<UTF_8>(it, end)) ||
            (i == 3 && (*it & 0xF0) == 0xE0 && IsValidUTF<UTF_8>(it, end)) ||
            (i == 4 && (*it & 0xF8) == 0xF0 && IsValidUTF<UTF_8>(it, end))) {
            return it;
        }
    }

    return end;
}

template <typename T>
constexpr T PriorUTF16(T end, T begin) {
    static_assert(type::is_bidirectional_iterator<T>::value, "Value should be a bidirectional iterator");
    static_assert(std::is_integral<type::iterator_underlying_type_t<T>>::value,
                  "Iterator internal type should be an integer");
    static_assert(sizeof(type::iterator_underlying_type_t<T>) == sizeof(char16),
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

template <typename T>
constexpr T PriorUTF32(T end, T begin) {
    static_assert(type::is_bidirectional_iterator<T>::value, "Value should be a bidirectional iterator");
    static_assert(std::is_integral<type::iterator_underlying_type_t<T>>::value,
                  "Iterator internal type should be an integer");
    static_assert(sizeof(type::iterator_underlying_type_t<T>) == sizeof(char32),
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
template <typename T, typename>
constexpr CodeUnit<Base>::CodeUnit(T codePoint) : m_unit() {
    if constexpr (Base == UTF_8) {
        *this = std::move(internal::CodePointToUTF8(codePoint));
    } else if constexpr (Base == UTF_16) {
        *this = std::move(internal::CodePointToUTF16(codePoint));
    } else if constexpr (Base == UTF_32) {
        *this = std::move(internal::CodePointToUTF32(codePoint));
    }
}

template <Encoding Base>
template <typename T>
constexpr CodeUnit<Base>::CodeUnit(const T* begin, const T* end) : CodeUnit(begin, (end - begin)) {}

template <Encoding Base>
template <typename T>
constexpr CodeUnit<Base>::CodeUnit(const T* begin, const size_t size) {
    static_assert(sizeof(T) == size_t(Base), "Error invalid value, it should has the same value in bits as Base");

    ENGINE_ASSERT(IsValidUTF<Base>(begin, begin + size), "Invalid UTF code unit");

    if constexpr (Base == UTF_8) {
        ENGINE_ASSERT(size <= 4, "UTF-8 should have at least four 8 bit values");
    } else if constexpr (Base == UTF_16) {
        ENGINE_ASSERT(size <= 2, "UTF-16 should have at least two 16 bit values");
    } else if constexpr (Base == UTF_32) {
        ENGINE_ASSERT(size <= 1, "UTF-32 should have at least one 32 bit value");
    }

    m_unit.fill(0x0);
    if (size != 0) {
        std::memcpy(m_unit.data(), begin, sizeof(CodeUnit::value_type) * size);
    }
}

template <Encoding Base>
constexpr char32 CodeUnit<Base>::getCodePoint() const {
    const auto* begin = m_unit.data();
    const auto* end = m_unit.data() + getSize();
    return CodePointFromUTF<Base>(begin, end);
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
constexpr size_t CodeUnit<Base>::getSize() const {
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

template <Encoding Base, typename T>
constexpr CodeUnitRange<Base, T>::CodeUnitRange() : m_range(nullptr, nullptr) {}

template <Encoding Base, typename T>
constexpr CodeUnitRange<Base, T>::CodeUnitRange(std::pair<pointed_type, pointed_type>&& range)
      : m_range(std::move(range)) {}

template <Encoding Base, typename T>
constexpr CodeUnitRange<Base, T>::CodeUnitRange(pointed_type begin, pointed_type end) : m_range(begin, end) {}

template <Encoding Base, typename T>
constexpr CodeUnit<Base> CodeUnitRange<Base, T>::get() const {
    return CodeUnit<Base>(m_range.first, m_range.second);
}

template <Encoding BaseFrom, typename T>
template <Encoding BaseTo>
constexpr CodeUnit<BaseTo> CodeUnitRange<BaseFrom, T>::getAs() const {
    if constexpr (BaseFrom == BaseTo) {
        return CodeUnit<BaseFrom>(m_range.first, m_range.second);
    } else {
        return CodeUnit<BaseTo>(CodePointFromUTF<BaseFrom>(m_range.first, m_range.second));
    }
}

template <Encoding Base, typename T>
constexpr typename CodeUnitRange<Base, T>::pointed_type CodeUnitRange<Base, T>::begin() const {
    return m_range.first;
}

template <Encoding Base, typename T>
constexpr typename CodeUnitRange<Base, T>::pointed_type CodeUnitRange<Base, T>::end() const {
    return m_range.second;
}

template <Encoding Base, typename T>
constexpr auto CodeUnitRange<Base, T>::getRange() const -> const std::pair<pointed_type, pointed_type>& {
    return m_range;
}

template <Encoding Base, typename T>
template <typename U>
constexpr bool CodeUnitRange<Base, T>::operator==(const CodeUnitRange<Base, U>& other) const {
    size_t size = (m_range.second - m_range.first);
    size_t otherSize = (other.m_range.second - other.m_range.first);
    return otherSize == size && memcmp(m_range.first, other.m_range.first, size) == 0;
}

template <Encoding Base, typename T>
template <typename U>
constexpr bool CodeUnitRange<Base, T>::operator!=(const CodeUnitRange<Base, U>& other) const {
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
constexpr Iterator<Base, T> Iterator<Base, T>::operator+(uint32 num) {
    // TODO: Improve this iteration for edge cases
    // Check if already on the end
    const auto* newBegin = m_ref.begin();
    for (uint32 i = 0; i < num; i++) {
        const auto* it = NextUTF<Base>(newBegin, m_maxRange.second);
        if (it == newBegin) {
            // Error getting next
            return Iterator(m_maxRange, m_maxRange.second + 1);
        }
        newBegin = it;
    }
    return Iterator(m_maxRange, newBegin);
}

template <Encoding Base, typename T>
constexpr Iterator<Base, T>& Iterator<Base, T>::operator+=(uint32 num) {
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
constexpr Iterator<Base, T> Iterator<Base, T>::operator-(uint32 num) {
    // TODO: Improve this iteration for edge cases
    // Check if already on the end
    const auto* newBegin = m_ref.begin();
    for (uint32 i = 0; i < num; i++) {
        const auto* it = PriorUTF<Base>(newBegin, m_maxRange.first);
        if (it == newBegin) {
            // Error getting next
            return Iterator(m_maxRange, m_maxRange.first);
        }
        newBegin = it;
    }
    return Iterator(m_maxRange, newBegin);
}

template <Encoding Base, typename T>
constexpr Iterator<Base, T>& Iterator<Base, T>::operator-=(uint32 num) {
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
    return m_ref.getRange() > other.m_ref.getRange();
}

template <Encoding Base, typename T>
constexpr bool Iterator<Base, T>::operator<=(const Iterator& other) const {
    return *this < other || *this == other;
}

template <Encoding Base, typename T>
constexpr bool Iterator<Base, T>::operator>=(const Iterator& other) const {
    return *this > other || *this == other;
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

template <Encoding BaseFrom, Encoding BaseTo, typename T, typename Ret, typename>
constexpr void UtfToUtf(T begin, T end, std::basic_string<Ret>* result) {
    size_t sizeBytes = 0;
    CodeUnit<BaseTo> hola(0x0);

    ForEachUTF<BaseFrom>(begin, end, [&sizeBytes](auto codeUnitRange) {
        auto convertedCodeUnit = codeUnitRange.template getAs<BaseTo>();
        sizeBytes += convertedCodeUnit.getSize();
    });

    result->reserve(result->size() + sizeBytes);

    ForEachUTF<BaseFrom>(begin, end, [&result](auto codeUnitRange) {
        auto convertedCodeUnit = codeUnitRange.template getAs<BaseTo>();
        for (auto val : convertedCodeUnit) {
            *result += val;
        }
    });
}

template <Encoding Base, typename T>
constexpr char32 CodePointFromUTF(T begin, T end) {
    if constexpr (Base == UTF_8) {
        return internal::CodePointFromUTF8(begin, end);
    } else if constexpr (Base == UTF_16) {
        return internal::CodePointFromUTF16(begin, end);
    } else if constexpr (Base == UTF_32) {
        return internal::CodePointFromUTF32(begin, end);
    }
}

template <Encoding Base, typename T>
constexpr T NextUTF(T begin, T end) {
    if constexpr (Base == UTF_8) {
        return internal::NextUTF8(begin, end);
    } else if constexpr (Base == UTF_16) {
        return internal::NextUTF16(begin, end);
    } else if constexpr (Base == UTF_32) {
        return internal::NextUTF32(begin, end);
    }
}

template <Encoding Base, typename T>
constexpr T PriorUTF(T end, T begin) {
    if constexpr (Base == UTF_8) {
        return internal::PriorUTF8(end, begin);
    } else if constexpr (Base == UTF_16) {
        return internal::PriorUTF16(end, begin);
    } else if constexpr (Base == UTF_32) {
        return internal::PriorUTF32(end, begin);
    }
}

template <Encoding Base, typename T, typename Func>
constexpr T ForEachUTF(T begin, T end, Func fn) {
    T s = begin;

    while (s < end) {
        auto next = NextUTF<Base>(s, end);
        if (next == s) {
            // Error encoding
            return next;
        }
        fn(CodeUnitRange<Base, decltype(s)>(s, next));
        s = next;
    }

    return end;
}

template <Encoding Base, typename T>
constexpr size_t GetSizeUTF(T begin, T end) {
    size_t size = 0;
    auto it = ForEachUTF<Base>(begin, end, [&size](auto /*unused*/) { size++; });
    if (it != end) {
        size = -1;
    }
    return size;
}

template <Encoding Base, typename T>
constexpr bool IsValidUTF(T begin, T end) {
    return ForEachUTF<Base>(begin, end, [](auto /*unused*/) {}) == end;
}

}  // namespace utf

}  // namespace engine

// namespace std {

// template <typename Func, Encoding Base, typename T>
// constexpr decltype(auto) apply(Func&& func, engine::utf::CodeUnit<Base> t) {
//     auto [begin, end] = t;
//     return std::invoke(std::move(func), begin, end);
// }

// }  // namespace std
