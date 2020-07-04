#pragma once

#include <Util/Prerequisites.hpp>

#include <Util/TypeTraits.hpp>

#include <array>
#include <string>

namespace engine {

namespace utf {

/**
 * @brief Class to handle a single UTF code unit
 *
 * @tparam Base The base for the UTF support, must be 8, 16 or 32
 */
template <size_t Base>
class CodeUnit {
public:
    static_assert((Base == 8) || (Base == 16) || (Base == 32), "Error invalid Base, it should be either 8, 16 or 32");

    /**
     * @brief Type of the internal data for the code unit
     */
    using value_type = std::conditional_t<(Base == 8), uint8, std::conditional_t<(Base == 16), uint16, uint32>>;

    /**
     * @brief Type of container of the complete code unit data
     */
    using data_type = std::array<value_type, 32 / (sizeof(value_type) * 8)>;

    /**
     * @brief Create an unit from an array
     *
     * @param data The array to fill the code unit
     */
    explicit constexpr CodeUnit(data_type data);

    /**
     * @brief Creates a new code unit from an Unicode code point
     *
     * @param codePoint The Unicode code point to convert
     *
     * @tparam The type of the codePoint,
     * @tparam SFINAE to check T is integer and have 4 bytes of size
     */
    template <typename T = char32, typename = std::enable_if_t<std::is_integral_v<T> && sizeof(T) == 4>>
    explicit constexpr CodeUnit(T codePoint);

    /**
     * @brief Create an unit from a range
     *
     * The range must follow the following restrictions:
     * - UTF-8: (end - begin) <= 4
     * - UTF-16: (end - begin) <= 2
     * - UTF-32: (end - begin) <= 1
     *
     * If `(end == begin)` the code point is considered to be null
     *
     * @param begin The begin of the iterator
     * @param end The end of the iterator
     *
     * @tparam T The type used for the UTF internal data, it must be sizeof(T) equals:
     *           1 byte for UTF-8, 2 byte for UTF-16, and 4 bytes for UTF-32
     */
    template <typename T>
    explicit constexpr CodeUnit(const T* begin, const T* end);

    /**
     * @brief Create an unit from a range
     *
     * The range must follow the following restrictions:
     * - UTF-8: `size <= 4`
     * - UTF-16: `size <= 2`
     * - UTF-32: `size <= 1`
     *
     * If `size` is 0 the code point is considered to be null
     *
     * @param begin The begin of the iterator
     * @param size The size of the iterator
     *
     * @tparam T The type used for the UTF internal data, it must be sizeof(T) equals:
     *           1 byte for UTF-8, 2 byte for UTF-16, and 4 bytes for UTF-32
     */
    template <typename T>
    explicit constexpr CodeUnit(const T* begin, size_t size);

    /**
     * @brief Copy contructor
     *
     * @param other Other to copy from
     */
    constexpr CodeUnit(const CodeUnit& other) = default;

    /**
     * @brief Move contructor
     *
     * @param other Other to move to this
     */
    constexpr CodeUnit(CodeUnit&& other) = default;

    /**
     * @brief Copy operator
     *
     * @param other Other to copy from
     * @return Reference to this
     */
    constexpr CodeUnit& operator=(const CodeUnit& other) = default;

    /**
     * @brief Move operator
     *
     * @param other Other to move to this
     * @return Reference to this
     */
    constexpr CodeUnit& operator=(CodeUnit&& other) = default;

    /**
     * @brief Get the Unicode code point
     *
     * @return The Unicode code point
     */
    constexpr char32 getCodePoint() const;

    /**
     * @brief Returns the begining of the code unit
     *
     * @return Constant iterator
     */
    constexpr const value_type* begin() const;

    /**
     * @brief Returns the end of the code unit
     *
     * @return Constant iterator
     */
    constexpr const value_type* end() const;

    /**
     * @brief Get the internal data object
     *
     * @return Array with the UTF data
     */
    constexpr const data_type& getData() const;

    /**
     * @brief Returns the size that the current code unit uses to store certain Unicode code point
     *
     * @return The size
     */
    constexpr size_t getSize() const;

    /**
     * @brief Check equality
     *
     * @param right Another CodeUnit to check
     * @return true If equals, false otherwhise
     */
    constexpr bool operator==(const CodeUnit& right) const;

    /**
     * @brief Check equality
     *
     * @param right data_type to check
     * @return true If equals, false otherwhise
     */
    constexpr bool operator==(const data_type& right) const;

    /**
     * @brief Check equality
     *
     * @param right Check if CodeUnit is equals an ASCII char
     * @return true If equals, false otherwhise
     */
    constexpr bool operator==(char8 right) const;

private:
    data_type m_unit;
};

/**
 * @brief Convert between UTF-8, UTF-16 and UTF-32
 *
 * This method will append to the `result` string the requested Base for the conversion
 *
 * @tparam BaseFrom The base to convert from, must be 8, 16, or 32, for UTF-8, UTF-16 and UTF-32 respectively, and
 *                  different than BaseTo
 * @tparam BaseTo The base to convert to, must be 8, 16, or 32, for UTF-8, UTF-16 and UTF-32 respectively,
 *                and different than BaseFrom
 * @tparam T The type of the iterator has the string
 * @tparam Ret The type of the string data, must be 8, 16, or 32, for UTF-8, UTF-16 and UTF-32 respectively
 * @tparam std::enable_if_t<sizeof(Ret) == (BaseTo / 8)> SFINAE
 * @param begin The begin of the string to convert from
 * @param end The end of the string to convert from
 * @param result The string to modify
 */
template <size_t BaseFrom,
          size_t BaseTo,
          typename T,
          typename Ret,
          typename = std::enable_if_t<BaseFrom != BaseTo && sizeof(Ret) == (BaseTo / 8)>>
constexpr void UtfToUtf(T begin, T end, std::basic_string<Ret>* result);

/**
 * @brief Get a Unicode code point from a code unit
 *
 * @tparam Base Must be 8, 16, or 32, for UTF-8, UTF-16 and UTF-32 respectively
 * @tparam T The type of the iterator has the string
 * @param begin The start of the code unit
 * @param end The end of the code unit
 * @return Unicode code point
 */
template <size_t Base, typename T>
constexpr char32 CodePointFromUTF(T begin, T end);

/**
 * @brief Get the iterator to the next code unit of the UTF string
 *
 * @tparam Base Must be 8, 16, or 32, for UTF-8, UTF-16 and UTF-32 respectively
 * @tparam T The type of the iterator has the string
 * @param begin The start of the iterator to begin searching
 * @param end The iterator to the end of the UTF string
 * @return The iterator to the next character, `end` if the end of the string has been reached
 *         or `begin` if a parsing error has occurred reading the UTF string
 */
template <size_t Base, typename T>
constexpr T NextUTF(T begin, T end);

/**
 * @brief Get the iterator to the prior code unit of the UTF string
 *
 * @tparam Base Must be 8, 16, or 32, for UTF-8, UTF-16 and UTF-32 respectively
 * @tparam T The type of the iterator has the string
 * @param end The start of the iterator to begin searching backwards
 * @param begin The iterator to the start of the UTF string
 * @return The iterator to the prior character, `begin` if the start of the string has been reached
 *         or `end` if a parsing error has occurred reading the UTF string
 */
template <size_t Base, typename T>
constexpr T PriorUTF(T end, T begin);

/**
 * @brief Iterate over an UTF string
 *
 * @tparam Base Must be 8, 16, or 32, for UTF-8, UTF-16 and UTF-32 respectively
 * @tparam T The type of the iterator has the string
 * @param end The iterator to the start of the UTF string
 * @param begin The iterator to the end of the UTF string
 */

/**
 * @brief
 *
 * @tparam Base Must be 8, 16, or 32, for UTF-8, UTF-16 and UTF-32 respectively
 * @tparam T The type of the iterator has the string
 * @tparam Func The function of type `void(T begin, T end)`
 * @param begin The iterator to the start of the UTF string
 * @param end The iterator to the end of the UTF string
 * @param fn Function to callback each time a new code unit is found
 * @return `end` if the iteration completed successfully, or another iterator to
 *         the internal sequence if an encoding error occured.
 */
template <size_t Base, typename T, typename Func>
constexpr T ForEachUTF(T begin, T end, Func fn);

/**
 * @brief Get the number of code units in the UTF string
 *
 * @note This method does not check for a valid UTF string
 *
 * @tparam Base Must be 8, 16, or 32, for UTF-8, UTF-16 and UTF-32 respectively
 * @tparam T The type of the iterator has the string
 * @param begin The iterator to the start of the UTF string
 * @param end The iterator to the end of the UTF string
 * @return The number of code units that the UTF string has
 */
template <size_t Base, typename T>
constexpr size_t GetSizeUTF(T begin, T end);

/**
 * @brief Validate an UTF string
 *
 * @tparam Base Must be 8, 16, or 32, for UTF-8, UTF-16 and UTF-32 respectively
 * @tparam T The type of the iterator has the string
 * @param begin The iterator to the start of the UTF string
 * @param end The iterator to the end of the UTF string
 * @return true If is valid, false otherwise
 */
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

// namespace std {

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

// }  // namespace std

#include <Util/UTF.inl>
