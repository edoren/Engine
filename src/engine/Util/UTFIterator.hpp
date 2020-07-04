#pragma once

#include <Util/Prerequisites.hpp>

#include <Util/UTF.hpp>

#include <iterator>
#include <utility>

namespace engine {

class String;
class StringView;

/**
 * @brief Read-only iterator class for an UTF sequence
 *
 * @tparam Base The base for the UTF support, must be 8, 16 or 32
 * @tparam T The internal type for the unit pointer, by default char8, char16 or char32 for Base 8, 16 and 32
 *           respectively
 */
template <size_t Base,
          typename T = std::conditional_t<(Base == 8), char8, std::conditional_t<(Base == 16), char16, char32>>>
class ENGINE_API UTFIterator {
    static_assert((Base == 8) || (Base == 16) || (Base == 32), "Error invalid Base, it should be either 8, 16 or 32");
    static_assert(std::is_integral<T>::value, "Pointed type should be an integer");
    static_assert(sizeof(T) * 8 == Base, "Error invalid type (T), it should has the same size in bits as Base");

    friend class String;
    friend class StringView;

public:
    using pointed_type = const T*;  ///< The type of the pointed element

    /** @brief Helper class that holds the begin and end a pointer to a utf::CodeUnit */
    class CodeUnitRange {
        friend class UTFIterator;

    public:
        /**
         * @brief Get the target code unit
         *
         * @return A code unit with of the pointed range
         */
        constexpr utf::CodeUnit<Base> getUnit() const;

    private:
        constexpr CodeUnitRange(std::pair<pointed_type, pointed_type>&& value);
        constexpr CodeUnitRange(const CodeUnitRange& other) = default;
        constexpr CodeUnitRange(CodeUnitRange&& other) = default;
        constexpr CodeUnitRange& operator=(const CodeUnitRange& other) = default;
        constexpr CodeUnitRange& operator=(CodeUnitRange&& other) = default;

        std::pair<pointed_type, pointed_type> m_value;
    };

    using difference_type = std::ptrdiff_t;                     ///< The difference type
    using value_type = CodeUnitRange;                           ///< The value type
    using pointer = value_type*;                                ///< The pointer type
    using reference = value_type&;                              ///< The reference type
    using iterator_category = std::bidirectional_iterator_tag;  ///< The category of the iterator

    /**
     * @brief Copy constructor
     *
     * @param other The other iterator to copy
     */
    constexpr UTFIterator(const UTFIterator& other) = default;

    /**
     * @brief Move constructor
     *
     * @param other The other iterator to move into
     */
    constexpr UTFIterator(UTFIterator&& other) = default;

    /** @brief Destructor */
    ~UTFIterator() = default;

    /**
     * @brief Copy operator
     *
     * @param other The other iterator to copy
     * @return Reference to this
     */
    constexpr UTFIterator& operator=(const UTFIterator& other) = default;

    /**
     * @brief Move operator
     *
     * @param other The other iterator to move into
     * @return Reference to this
     */
    constexpr UTFIterator& operator=(UTFIterator&& other) = default;

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
    constexpr UTFIterator operator+(uint32 num);

    /**
     * @brief Addition assignment operator
     *
     * @param num The number to increase
     * @return The current iterator pointing a next code unit
     */
    constexpr UTFIterator& operator+=(uint32 num);

    /**
     * @brief Pre-increment operator
     *
     * @return The current iterator pointing to the next code unit
     */
    constexpr UTFIterator& operator++();

    /**
     * @brief Post-increment operator
     *
     * @return A new iterator pointing to the current code unit
     */
    constexpr UTFIterator operator++(int);

    /**
     * @brief Subtraction operator
     *
     * @param num The number to decrease
     * @return A new iterator pointing a previous code unit
     */
    constexpr UTFIterator operator-(uint32 num);

    /**
     * @brief Subtraction assignment operator
     *
     * @param num The number to decrease
     * @return The current iterator pointing a previous code unit
     */
    constexpr UTFIterator& operator-=(uint32 num);

    /**
     * @brief Pre-decrement operator
     *
     * @return The current iterator pointing to the previous code unit
     */
    constexpr UTFIterator& operator--();

    /**
     * @brief Post-decrement operator
     *
     * @return A new iterator pointing to the current code unit
     */
    constexpr UTFIterator operator--(int);

    /**
     * @brief Less than operator
     *
     * @return true if condition satisfies, false otherwise
     */
    constexpr bool operator<(const UTFIterator& other) const;

    /**
     * @brief Greater than operator
     *
     * @return true if condition satisfies, false otherwise
     */
    constexpr bool operator>(const UTFIterator& other) const;

    /**
     * @brief Less than or equal to operator
     *
     * @return true if condition satisfies, false otherwise
     */
    constexpr bool operator<=(const UTFIterator& other) const;

    /**
     * @brief Greater than or equal to operator
     *
     * @return true if condition satisfies, false otherwise
     */
    constexpr bool operator>=(const UTFIterator& other) const;

    /**
     * @brief Equal to operator
     *
     * @return true if condition satisfies, false otherwise
     */
    constexpr bool operator==(const UTFIterator& other) const;

    /**
     * @brief Not equal to operator
     *
     * @return true if condition satisfies, false otherwise
     */
    constexpr bool operator!=(const UTFIterator& other) const;

    /**
     * @brief Pointer to the start of the pointed code unit
     *
     * @return Pointer to the start of the code unit
     */
    constexpr UTFIterator<Base, T>::pointed_type getPtr() const;

private:
    constexpr UTFIterator(std::pair<pointed_type, pointed_type> maxRange, pointed_type begin);

    std::pair<pointed_type, pointed_type> m_maxRange;
    value_type m_ref;
};

}  // namespace engine

#include <Util/UTFIterator.inl>
