#pragma once

#include <Util/Prerequisites.hpp>

#include <Util/TypeTraits.hpp>

#include <string>

namespace engine {

namespace utf {

template <size_t Base,
          typename T,
          typename Ret,
          typename = std::enable_if_t<std::is_integral<T>::value && (sizeof(T) == sizeof(char32)) &&
                                      (sizeof(Ret) * 8 == Base)>>
constexpr void AppendToUtf(T codePoint, std::basic_string<Ret>* result);

template <size_t BaseFrom,
          size_t BaseTo,
          typename T,
          typename Ret,
          typename = std::enable_if_t<sizeof(Ret) == (BaseTo / 8)>>
constexpr void UtfToUtf(T begin, T end, std::basic_string<Ret>* result);

template <size_t Base, typename T>
constexpr char32 CodePointFromUTF(T begin, T end);

template <size_t Base, typename T>
constexpr std::pair<char32, size_t> CodePointToUTF(T codePoint);

template <size_t Base, typename T>
constexpr T NextUTF(T begin, T end);

template <size_t Base, typename T>
constexpr T PriorUTF(T begin, T end);

template <size_t Base, typename T, typename Func>
constexpr T ForEachUTF(T begin, T end, Func fn);

template <size_t Base, typename T>
constexpr size_t GetSizeUTF(T begin, T end);

template <size_t Base, typename T>
constexpr bool IsValidUTF(T begin, T end);

}  // namespace utf

template <size_t Base, typename T>
struct UTFCodeUnitRange {
    static_assert((Base == 8) || (Base == 16) || (Base == 32), "Error invalid Base, it should be either 8, 16 or 32");
    static_assert(sizeof(std::remove_pointer_t<T>) * 8 == Base,
                  "Error invalid value (T), it should has the same value in bits as Base");

    explicit constexpr UTFCodeUnitRange(T begin, T end) {
        if (utf::IsValidUTF<Base>(begin, end)) {
            this->begin = begin;
            this->end = end;
        }
    }

    explicit constexpr UTFCodeUnitRange(T begin, const std::pair<T, T>& maxRange) : begin(begin) {
        end = begin;

        if (maxRange.second == begin) {  // End of iterator
            return;
        }

        if constexpr (Base == 8) {
            if ((*begin & 0x80) == 0x00) {
                end += 1;
            } else if ((*begin & 0xE0) == 0xC0) {
                end += 2;
            } else if ((*begin & 0xF0) == 0xE0) {
                end += 3;
            } else if ((*begin & 0xF8) == 0xF0) {
                end += 4;
            }
        } else if constexpr (Base == 16) {
            if ((*begin >= 0x0000 && *begin <= 0xD7FF) || (*begin >= 0xE000 && *begin <= 0xFFFF)) {
                end += 1;
            } else if (*begin >= 0x010000 && *begin <= 0x10FFFF) {
                end += 2;
            }
        } else if constexpr (Base == 32) {
            end += 1;
        }
    }

    constexpr char32 getCodePoint() const {
        return utf::CodePointFromUTF<Base>(begin, end);
    }

    auto getUnit() const {
        return utf::CodePointToUTF<Base>(begin, end);
    }

    bool operator==(const UTFCodeUnitRange& other) const {
        return begin == other.begin && end == other.end;
    }

    bool operator!=(const UTFCodeUnitRange& other) const {
        return !(*this == other);
    }

    T begin;
    T end;
};

template <size_t I, typename T>
auto& get(engine::UTFCodeUnitRange<8, T>& cp) noexcept {
    if constexpr (I == 0) {
        return cp.begin;
    } else if constexpr (I == 1) {
        return cp.end;
    }
}

template <size_t I, typename T>
const auto& get(const engine::UTFCodeUnitRange<8, T>& cp) noexcept {
    if constexpr (I == 0) {
        return cp.begin;
    } else if constexpr (I == 1) {
        return cp.end;
    }
}

template <size_t I, typename T>
auto&& get(engine::UTFCodeUnitRange<8, T>&& cp) noexcept {
    if constexpr (I == 0) {
        return cp.begin;
    } else if constexpr (I == 1) {
        return cp.end;
    }
}

}  // namespace engine

namespace std {

template <size_t Base, typename T>
struct tuple_size<engine::UTFCodeUnitRange<Base, T>> : std::integral_constant<std::size_t, 2> {};

template <size_t Base, typename T>
struct tuple_element<0, engine::UTFCodeUnitRange<Base, T>> {
    using type = T;
};

template <size_t Base, typename T>
struct tuple_element<1, engine::UTFCodeUnitRange<Base, T>> {
    using type = T;
};

template <typename Func, size_t Base, typename T>
constexpr decltype(auto) apply(Func&& func, engine::UTFCodeUnitRange<Base, T> t) {
    auto [begin, end] = t;
    return std::invoke(std::move(func), begin, end);
}

}  // namespace std

#include <Util/UTF.inl>
