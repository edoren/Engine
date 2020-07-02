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
constexpr CodeUnit<8> CodePointToUTF8(T codePoint) {
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

    return CodeUnit<8>(codeUnit.data(), size);
}

template <typename T>
constexpr CodeUnit<16> CodePointToUTF16(T codePoint) {
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

    return CodeUnit<16>(codeUnit.data(), size);
}

template <typename T>
constexpr CodeUnit<32> CodePointToUTF32(T codePoint) {
    static_assert(std::is_integral<T>::value, "Code point should be an integer");
    static_assert(sizeof(T) == sizeof(char32), "Code point has an invalid size");

    return CodeUnit<32>(&codePoint, 1);
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
    if ((*s & 0x80) == 0x00) {
        // 1-byte ascii (began with 0b0xxxxxxx)
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
    if ((*s >= 0x0000 && *s <= 0xD7FF) || (*s >= 0xE000 && *s <= 0xFFFF)) {
        return s + 1;
    }
    // Check the first 16 bit code unit to be 0b110110yyyyyyyyyy
    // Check the second 16 bit code unit to be 0b110111xxxxxxxxxx
    if ((*s & 0xFC00) == 0xD800 && (*(s + 1) & 0xFC00) == 0xDC00) {
        // Verify out of range
        if ((s + 2) > end) {
            return s;
        }

        return s + 2;
    }
    // We have an invalid UTF-16 code point ent
    return s;
}

template <typename T>
constexpr T NextUTF32(T begin, T /*end*/) {
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
        if ((i == 1 && (*it & 0x80) == 0x00 && IsValidUTF<8>(it, end)) ||
            (i == 2 && (*it & 0xE0) == 0xC0 && IsValidUTF<8>(it, end)) ||
            (i == 3 && (*it & 0xF0) == 0xE0 && IsValidUTF<8>(it, end)) ||
            (i == 4 && (*it & 0xF8) == 0xF0 && IsValidUTF<8>(it, end))) {
            return it;
        }
    }

    return begin;
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

    return begin;
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

template <size_t Base, typename T, typename Ret, typename>
constexpr void AppendToUtf(T codePoint, std::basic_string<Ret>* result) {
    auto codeUnit = CodePointToUTF<Base>(codePoint);
    result->reserve(result->size() + codeUnit.getSize());
    for (auto val : codeUnit) {
        *result += val;
    }
}

template <size_t BaseFrom, size_t BaseTo, typename T, typename Ret, typename>
constexpr void UtfToUtf(T begin, T end, std::basic_string<Ret>* result) {
    size_t sizeBytes = 0;
    result->clear();

    ForEachUTF<BaseFrom>(begin, end, [&sizeBytes](auto codeUnitBegin, auto codeUnitEnd) {
        auto codePoint = CodePointFromUTF<BaseFrom>(codeUnitBegin, codeUnitEnd);
        auto codeUnit = CodePointToUTF<BaseTo>(codePoint);
        sizeBytes += codeUnit.getSize();
    });

    result->reserve(sizeBytes);

    ForEachUTF<BaseFrom>(begin, end, [&result](auto codeUnitBegin, auto codeUnitEnd) {
        auto codePoint = CodePointFromUTF<BaseFrom>(codeUnitBegin, codeUnitEnd);
        auto codeUnit = CodePointToUTF<BaseTo>(codePoint);
        for (auto val : codeUnit) {
            *result += val;
        }
    });
}

template <size_t Base, typename T>
constexpr char32 CodePointFromUTF(T begin, T end) {
    if constexpr (Base == 8) {
        return internal::CodePointFromUTF8(begin, end);
    } else if constexpr (Base == 16) {
        return internal::CodePointFromUTF16(begin, end);
    } else if constexpr (Base == 32) {
        return internal::CodePointFromUTF32(begin, end);
    } else {
        static_assert((Base == 8) || (Base == 16) || (Base == 32),
                      "Error invalid Base, it should be either 8, 16 or 32");
    }
}

template <size_t Base, typename T>
constexpr CodeUnit<Base> CodePointToUTF(T codePoint) {
    if constexpr (Base == 8) {
        return internal::CodePointToUTF8(codePoint);
    } else if constexpr (Base == 16) {
        return internal::CodePointToUTF16(codePoint);
    } else if constexpr (Base == 32) {
        return internal::CodePointToUTF32(codePoint);
    } else {
        static_assert((Base == 8) || (Base == 16) || (Base == 32),
                      "Error invalid Base, it should be either 8, 16 or 32");
    }
}

template <size_t Base, typename T>
constexpr T NextUTF(T begin, T end) {
    if constexpr (Base == 8) {
        return internal::NextUTF8(begin, end);
    } else if constexpr (Base == 16) {
        return internal::NextUTF16(begin, end);
    } else if constexpr (Base == 32) {
        return internal::NextUTF32(begin, end);
    } else {
        static_assert((Base == 8) || (Base == 16) || (Base == 32),
                      "Error invalid Base, it should be either 8, 16 or 32");
    }
}

template <size_t Base, typename T>
constexpr T PriorUTF(T begin, T end) {
    if constexpr (Base == 8) {
        return internal::PriorUTF8(begin, end);
    } else if constexpr (Base == 16) {
        return internal::PriorUTF16(begin, end);
    } else if constexpr (Base == 32) {
        return internal::PriorUTF32(begin, end);
    } else {
        static_assert((Base == 8) || (Base == 16) || (Base == 32),
                      "Error invalid Base, it should be either 8, 16 or 32");
    }
}

template <size_t Base, typename T, typename Func>
constexpr T ForEachUTF(T begin, T end, Func fn) {
    T s = begin;

    while (s < end) {
        auto next = NextUTF<Base>(s, end);
        if (next == s) {
            // Error encoding
            return next;
        }
        fn(s, next);
        s = next;
    }

    return end;
}

template <size_t Base, typename T>
constexpr size_t GetSizeUTF(T begin, T end) {
    size_t size = 0;
    auto it = ForEachUTF<Base>(begin, end, [&size](auto /*unused*/, auto /*unused*/) { size++; });
    if (it != end) {
        size = -1;
    }
    return size;
}

template <size_t Base, typename T>
constexpr bool IsValidUTF(T begin, T end) {
    return ForEachUTF<Base>(begin, end, [](auto /*unused*/, auto /*unused*/) {}) == end;
}

template <size_t Base>
template <typename T>
constexpr CodeUnit<Base>::CodeUnit(const T* begin, const T* end) : CodeUnit(begin, (end - begin)) {}

template <size_t Base>
template <typename T>
constexpr CodeUnit<Base>::CodeUnit(const T* begin, const size_t size) {
    static_assert(sizeof(T) * 8 == Base, "Error invalid value, it should has the same value in bits as Base");

    ENGINE_ASSERT(IsValidUTF<Base>(begin, begin + size), "Invalid UTF code unit");

    if constexpr (Base == 8) {
        ENGINE_ASSERT(size > 0 && size <= 4, "UTF-8 should have at least four 8 bit values");
    } else if constexpr (Base == 16) {
        ENGINE_ASSERT(size > 0 && size <= 2, "UTF-16 should have at least two 16 bit values");
    } else if constexpr (Base == 32) {
        ENGINE_ASSERT(size > 0 && size <= 1, "UTF-32 should have at least one 32 bit value");
    }

    unit.fill(0x0);
    std::memcpy(unit.data(), begin, sizeof(CodeUnit::value_type) * size);
}

template <size_t Base>
constexpr char32 CodeUnit<Base>::getCodePoint() const {
    const auto* begin = unit.data();
    const auto* end = unit.data() + getSize();
    return utf::CodePointFromUTF<Base>(begin, end);
}

template <size_t Base>
constexpr const typename CodeUnit<Base>::value_type* CodeUnit<Base>::begin() const {
    return unit.data();
}

template <size_t Base>
constexpr const typename CodeUnit<Base>::value_type* CodeUnit<Base>::end() const {
    return begin() + getSize();
}

template <size_t Base>
constexpr const typename CodeUnit<Base>::data_type& CodeUnit<Base>::getData() const {
    return unit;
}

template <size_t Base>
constexpr size_t CodeUnit<Base>::getSize() const {
    for (size_t i = 0; i < unit.size(); i++) {
        if (unit[i] == 0x0) {
            return i;
        }
    }
    return unit.size();
}

template <size_t Base>
constexpr bool CodeUnit<Base>::operator==(const CodeUnit& right) const {
    return operator==(right.unit);
}

template <size_t Base>
constexpr bool CodeUnit<Base>::operator==(const data_type& right) const {
    return std::memcmp(unit.data(), right.data(), 4) == 0;
}

template <size_t Base>
constexpr bool CodeUnit<Base>::operator==(char8 right) const {
    return unit[0] == right;
}

template <size_t Base>
constexpr bool CodeUnit<Base>::operator!=(const CodeUnit& right) const {
    return !(*this == right);
}

}  // namespace utf

}  // namespace engine

namespace std {

// template <typename Func, size_t Base, typename T>
// constexpr decltype(auto) apply(Func&& func, engine::utf::CodeUnit<Base> t) {
//     auto [begin, end] = t;
//     return std::invoke(std::move(func), begin, end);
// }

}  // namespace std
