#pragma once

#include <Util/Prerequisites.hpp>

#include <Util/TypeTraits.hpp>

#include <array>
#include <string>

namespace engine {

namespace utf {

// @brief Class to manage
template <size_t Base>
class CodeUnit {
public:
    static_assert((Base == 8) || (Base == 16) || (Base == 32), "Error invalid Base, it should be either 8, 16 or 32");

    using value_type = std::conditional_t<(Base == 8), uint8, std::conditional_t<(Base == 16), uint16, uint32>>;
    using data_type = std::array<value_type, 32 / (sizeof(value_type) * 8)>;

    template <typename T>
    explicit constexpr CodeUnit(const T* begin, const T* end);

    template <typename T>
    explicit constexpr CodeUnit(const T* begin, size_t size);

    constexpr char32 getCodePoint() const;

    constexpr const value_type* begin() const;

    constexpr const value_type* end() const;

    constexpr const data_type& getData() const;

    constexpr size_t getSize() const;

    constexpr bool operator==(const CodeUnit& right) const;

    constexpr bool operator==(const data_type& right) const;

    constexpr bool operator==(char8 right) const;

    constexpr bool operator!=(const CodeUnit& right) const;

private:
    data_type unit;
};

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
constexpr CodeUnit<Base> CodePointToUTF(T codePoint);

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

// template <size_t I, typename T>
// auto& get(engine::utf::CodeUnit<8, T>& cp) noexcept;

// template <size_t I, typename T>
// const auto& get(const engine::utf::CodeUnit<8, T>& cp) noexcept;

// template <size_t I, typename T>
// auto&& get(engine::utf::CodeUnit<8, T>&& cp) noexcept;

}  // namespace utf

}  // namespace engine

namespace std {

// template <size_t Base, typename T>
// struct tuple_size<engine::utf::CodeUnit<Base, T>> : std::integral_constant<std::size_t, 2> {};

// template <size_t Base, typename T>
// struct tuple_element<0, engine::utf::CodeUnit<Base, T>> {
//     using type = T;
// };

// template <size_t Base, typename T>
// struct tuple_element<1, engine::utf::CodeUnit<Base, T>> {
//     using type = T;
// };

// template <typename Func, size_t Base, typename T>
// constexpr decltype(auto) apply(Func&& func, engine::utf::CodeUnit<Base, T> t);

}  // namespace std

#include <Util/UTF.inl>
