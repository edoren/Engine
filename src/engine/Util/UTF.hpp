#pragma once

#include <Util/TypeTraits.hpp>

#include <array>
#include <iterator>
#include <string>
#include <utility>

namespace engine {

namespace utf {

/**
 * @brief Enum to request certain UTF operations
 */
enum Encoding {
    UTF_8,   ///< Base for UTF-8 support.
    UTF_16,  ///< Base for UTF-16 support.
    UTF_32,  ///< Base for UTF-32 support.
};

/**
 * @brief Returns the size in bytes of each code unit internal data
 *
 * @param encoding The encoding to get the size from
 * @return This must return 1 for UTF-8, 2 for UTF-16 and 4 for UTF-32.
 */
constexpr size_t GetEncodingSize(Encoding encoding);

/**
 * @brief Class to handle a single UTF code unit
 *
 * @tparam Base The encoding for the UTF support. See @ref Encoding.
 */
template <Encoding Base>
class CodeUnit {
public:
    using size_type = size_t;  ///< The size type
    using value_type =
        std::conditional_t<(Base == UTF_8),
                           uint8,
                           std::conditional_t<(Base == UTF_16), uint16, uint32>>;  ///< Type of the internal data for
                                                                                   ///< the code unit
    using data_type =
        std::array<value_type, 32 / (sizeof(value_type) * 8)>;  ///< Type of container of the complete code unit data

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
     * @tparam T The type of the codePoint,
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
    explicit constexpr CodeUnit(const T* begin, size_type size);

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
    constexpr CodeUnit(CodeUnit&& other) noexcept = default;

    /**
     * @brief Copy operator
     *
     * @param other Other to copy from
     * @return Reference to self
     */
    constexpr CodeUnit& operator=(const CodeUnit& other) = default;

    /**
     * @brief Move operator
     *
     * @param other Other to move to this
     * @return Reference to self
     */
    constexpr CodeUnit& operator=(CodeUnit&& other) noexcept = default;

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
    constexpr size_type getSize() const;

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
 * @brief Helper class that holds the begin and end to an iterator/pointer of a CodeUnit
 *
 * @tparam Base The encoding for the UTF support. See @ref Encoding.
 * @tparam T The type of the iterator
 */
template <Encoding Base,
          typename T = std::conditional_t<(Base == UTF_8), char8, std::conditional_t<(Base == UTF_16), char16, char32>>>
class CodeUnitRange {
    static_assert(type::is_forward_iterator<T>::value, "Value should be a forward iterator");
    static_assert(std::is_integral<type::iterator_underlying_type_t<T>>::value,
                  "Iterator internal type should be an integer");
    static_assert(sizeof(type::iterator_underlying_type_t<T>) == GetEncodingSize(Base),
                  "Iterator internal type has an invalid size");

public:
    using size_type = size_t;  ///< The size type
    using pointed_type = T;    ///< The type of the pointed element

    /**
     * @brief Construct a default null initialized range
     */
    constexpr CodeUnitRange();

    /**
     * @brief Construct a new CodeUnitRange object
     *
     * @param range The begin and end of the code unit
     */
    constexpr CodeUnitRange(std::pair<pointed_type, pointed_type>&& range);

    /**
     * @brief Construct a new CodeUnitRange object
     *
     * @param begin The begin of the code unit
     * @param end The end of the code unit
     */
    constexpr CodeUnitRange(pointed_type begin, pointed_type end);

    /**
     * @brief Copy constructor
     *
     * @param other The other range to copy
     */
    constexpr CodeUnitRange(const CodeUnitRange& other) = default;

    /**
     * @brief Move constructor
     *
     * @param other The other range to move into
     */
    constexpr CodeUnitRange(CodeUnitRange&& other) noexcept = default;

    /**
     * @brief Copy operator
     *
     * @param other The other range to copy
     */
    constexpr CodeUnitRange& operator=(const CodeUnitRange& other) = default;

    /**
     * @brief Move operator
     *
     * @param other The other range to move into
     */
    constexpr CodeUnitRange& operator=(CodeUnitRange&& other) noexcept = default;

    /**
     * @brief Get the target code unit
     *
     * @return A code unit with of the pointed range
     */
    constexpr CodeUnit<Base> get() const;

    /**
     * @brief Get the target code unit
     *
     * @return A code unit with of the pointed range
     */
    template <Encoding BaseTo>
    constexpr CodeUnit<BaseTo> getAs() const;

    /**
     * @brief Get a pointer to the begin of the code unit
     *
     * @return A pointer to the begin of the code unit
     */
    constexpr pointed_type begin() const;

    /**
     * @brief Get a pointer to the end of the code unit
     *
     * @return A pointer to the end of the code unit
     */
    constexpr pointed_type end() const;

    /**
     * @brief Get the target code unit
     *
     * @return A code unit with of the pointed range
     */
    constexpr auto getRange() const -> const std::pair<pointed_type, pointed_type>&;

    /**
     * @brief Equal operator
     *
     * @return true if condition satisfies, false otherwise
     */
    template <typename U>
    constexpr bool operator==(const CodeUnitRange<Base, U>& other) const;

    /**
     * @brief Not equal operator
     *
     * @return true if condition satisfies, false otherwise
     */
    template <typename U>
    constexpr bool operator!=(const CodeUnitRange<Base, U>& other) const;

private:
    std::pair<pointed_type, pointed_type> m_range;
};

/**
 * @brief Read-only iterator class for an UTF sequence
 *
 * @tparam Base The encoding for the UTF support. See @ref Encoding.
 * @tparam T The internal type for the unit pointer, by default char8, char16 or char32 for UTF-8, UTF-16 and UTF-32
 *           respectively
 */
template <Encoding Base,
          typename T = std::conditional_t<(Base == UTF_8), char8, std::conditional_t<(Base == UTF_16), char16, char32>>>
class Iterator {
    static_assert(std::is_integral<T>::value, "Pointed type should be an integer");
    static_assert(sizeof(T) == GetEncodingSize(Base),
                  "Error invalid type (T), it should has the same size in bits as Base");

public:
    using size_type = size_t;                                   ///< The size type
    using difference_type = std::ptrdiff_t;                     ///< The difference type
    using value_type = CodeUnitRange<Base, const T*>;           ///< The value type
    using pointer = value_type*;                                ///< The pointer type
    using reference = value_type&;                              ///< The reference type
    using iterator_category = std::bidirectional_iterator_tag;  ///< The category of the iterator
    using pointed_type = typename value_type::pointed_type;     ///< @copydoc value_type::pointed_type

    /**
     * @brief Constructs a new Iterator object
     *
     * @param maxRange The continuos range the Iterator is working on
     * @param begin The start of the code unit
     */
    constexpr Iterator(std::pair<pointed_type, pointed_type> maxRange, pointed_type begin);

    /**
     * @brief Copy constructor
     *
     * @param other The other iterator to copy
     */
    constexpr Iterator(const Iterator& other) = default;

    /**
     * @brief Move constructor
     *
     * @param other The other iterator to move into
     */
    constexpr Iterator(Iterator&& other) noexcept = default;

    /**
     * @brief Destructor
     */
    ~Iterator() = default;

    /**
     * @brief Copy operator
     *
     * @param other The other iterator to copy
     * @return Reference to self
     */
    constexpr Iterator& operator=(const Iterator& other) = default;

    /**
     * @brief Move operator
     *
     * @param other The other iterator to move into
     * @return Reference to self
     */
    constexpr Iterator& operator=(Iterator&& other) noexcept = default;

    /**
     * @brief Dereference operator
     *
     * @return Reference to the internal code unit range
     */
    constexpr reference operator*();

    /**
     * @brief Dereference arrow operator
     *
     * @return Pointer to the internal code unit range
     */
    constexpr pointer operator->();

    /**
     * @brief Addition operator
     *
     * @param num The number to increase
     * @return A new iterator pointing a next code unit
     */
    constexpr Iterator operator+(uint32 num);

    /**
     * @brief Addition assignment operator
     *
     * @param num The number to increase
     * @return The current iterator pointing a next code unit
     */
    constexpr Iterator& operator+=(uint32 num);

    /**
     * @brief Pre-increment operator
     *
     * @return The current iterator pointing to the next code unit
     */
    constexpr Iterator& operator++();

    /**
     * @brief Post-increment operator
     *
     * @return A new iterator pointing to the current code unit
     */
    constexpr Iterator operator++(int);

    /**
     * @brief Subtraction operator
     *
     * @param num The number to decrease
     * @return A new iterator pointing a previous code unit
     */
    constexpr Iterator operator-(uint32 num);

    /**
     * @brief Subtraction assignment operator
     *
     * @param num The number to decrease
     * @return The current iterator pointing a previous code unit
     */
    constexpr Iterator& operator-=(uint32 num);

    /**
     * @brief Pre-decrement operator
     *
     * @return The current iterator pointing to the previous code unit
     */
    constexpr Iterator& operator--();

    /**
     * @brief Post-decrement operator
     *
     * @return A new iterator pointing to the current code unit
     */
    constexpr Iterator operator--(int);

    /**
     * @brief Less than operator
     *
     * @return true if condition satisfies, false otherwise
     */
    constexpr bool operator<(const Iterator& other) const;

    /**
     * @brief Greater than operator
     *
     * @return true if condition satisfies, false otherwise
     */
    constexpr bool operator>(const Iterator& other) const;

    /**
     * @brief Less than or equal operator
     *
     * @return true if condition satisfies, false otherwise
     */
    constexpr bool operator<=(const Iterator& other) const;

    /**
     * @brief Greater than or equal operator
     *
     * @return true if condition satisfies, false otherwise
     */
    constexpr bool operator>=(const Iterator& other) const;

    /**
     * @brief Equal operator
     *
     * @return true if condition satisfies, false otherwise
     */
    constexpr bool operator==(const Iterator& other) const;

    /**
     * @brief Not equal operator
     *
     * @return true if condition satisfies, false otherwise
     */
    constexpr bool operator!=(const Iterator& other) const;

    /**
     * @brief Pointer to the start of the pointed code unit
     *
     * @return Pointer to the start of the code unit
     */
    constexpr Iterator<Base, T>::pointed_type getPtr() const;

private:
    std::pair<pointed_type, pointed_type> m_maxRange;
    value_type m_ref;
};

/**
 * @brief Convert between UTF-8, UTF-16 and UTF-32
 *
 * This method will append to the `result` string the requested Base for the conversion
 *
 * @tparam BaseFrom The encoding to convert from. See @ref Encoding.
 * @tparam BaseTo The encoding to convert to. See @ref Encoding.
 * @tparam T The type of the iterator has the string
 * @tparam Ret The type of the string data, must be 8, 16, or 32, for UTF-8, UTF-16 and UTF-32 respectively
 * @param begin The begin of the string to convert from
 * @param end The end of the string to convert from
 * @param result The string to modify
 */
template <Encoding BaseFrom,
          Encoding BaseTo,
          typename T,
          typename Ret,
          typename = std::enable_if_t<BaseFrom != BaseTo && sizeof(Ret) == GetEncodingSize(BaseTo)>>
constexpr void UtfToUtf(T begin, T end, std::basic_string<Ret>* result);

/**
 * @brief Get a Unicode code point from a code unit
 *
 * @tparam Base The encoding for the UTF support. See @ref Encoding.
 * @tparam T The type of the iterator has the string
 * @param begin The start of the code unit
 * @param end The end of the code unit
 * @return Unicode code point
 */
template <Encoding Base, typename T>
constexpr char32 CodePointFromUTF(T begin, T end);

/**
 * @brief Get the iterator to the next code unit of the UTF string
 *
 * @tparam Base The encoding for the UTF support. See @ref Encoding.
 * @tparam T The type of the iterator has the string
 * @param begin The start of the iterator to begin searching
 * @param end The iterator to the end of the UTF string
 * @return The iterator to the next character, `end` if the end of the string has been reached
 *         or `begin` if a parsing error has occurred reading the UTF string
 */
template <Encoding Base, typename T>
constexpr T NextUTF(T begin, T end);

/**
 * @brief Get the iterator to the prior code unit of the UTF string
 *
 * @tparam Base The encoding for the UTF support. See @ref Encoding.
 * @tparam T The type of the iterator has the string
 * @param end The start of the iterator to begin searching backwards
 * @param begin The iterator to the start of the UTF string
 * @return The iterator to the prior character, `begin` if the start of the string has been reached
 *         or `end` if a parsing error has occurred reading the UTF string
 */
template <Encoding Base, typename T>
constexpr T PriorUTF(T end, T begin);

/**
 * @brief Iterate over an UTF string
 *
 * @tparam Base The encoding for the UTF support. See @ref Encoding.
 * @tparam T The type of the iterator has the string
 * @tparam Func The function of type `void(T begin, T end)`
 * @param begin The iterator to the start of the UTF string
 * @param end The iterator to the end of the UTF string
 * @param fn Function to callback each time a new code unit is found
 * @return `end` if the iteration completed successfully, or another iterator to
 *         the internal sequence if an encoding error occured.
 */
template <Encoding Base, typename T, typename Func>
constexpr T ForEachUTF(T begin, T end, Func fn);

/**
 * @brief Get the number of code units in the UTF string
 *
 * @note This method does not check for a valid UTF string
 *
 * @tparam Base The encoding for the UTF support. See @ref Encoding.
 * @tparam T The type of the iterator has the string
 * @param begin The iterator to the start of the UTF string
 * @param end The iterator to the end of the UTF string
 * @return The number of code units that the UTF string has
 */
template <Encoding Base, typename T>
constexpr size_t GetSizeUTF(T begin, T end);

/**
 * @brief Validate an UTF string
 *
 * @tparam Base The encoding for the UTF support. See @ref Encoding.
 * @tparam T The type of the iterator has the string
 * @param begin The iterator to the start of the UTF string
 * @param end The iterator to the end of the UTF string
 * @return true If is valid, false otherwise
 */
template <Encoding Base, typename T>
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

// template <Encoding Base, typename T>
// struct tuple_size<engine::utf::CodeUnit<Base, T>> : std::integral_constant<size_t, 2> {};

// template <Encoding Base, typename T>
// struct tuple_element<0, engine::utf::CodeUnit<Base, T>> {
//     using type = T;
// };

// template <Encoding Base, typename T>
// struct tuple_element<1, engine::utf::CodeUnit<Base, T>> {
//     using type = T;
// };

// template <typename Func, Encoding Base, typename T>
// constexpr decltype(auto) apply(Func&& func, engine::utf::CodeUnit<Base, T> t);

// }  // namespace std

#include <Util/UTF.inl>
