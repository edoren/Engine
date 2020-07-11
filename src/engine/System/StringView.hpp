#pragma once

#include <Util/Prerequisites.hpp>

#include <System/String.hpp>

namespace engine {

/**
 * @brief Utility class to handle a string view to a UTF-8 character sequence
 */
class ENGINE_API StringView {
public:
    ////////////////////////////////////////////////////////////
    // Types
    ////////////////////////////////////////////////////////////
    using size_type = size_t;                                              ///< Size type
    using const_iterator = utf::Iterator<utf::UTF_8, const char8*>;        ///< Read-only iterator type
    using iterator = const_iterator;                                       ///< Iterator type
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;  ///< Read-only reverse iterator type
    using reverse_iterator = std::reverse_iterator<iterator>;              ///< Reverse iterator type

    ////////////////////////////////////////////////////////////
    // Static member data
    ////////////////////////////////////////////////////////////
    static inline const size_type sInvalidPos = size_type(-1);  ///< Represents an invalid position in the string view

    /**
     * @brief Default constructor
     *
     * This constructor creates an empty string view.
     */
    constexpr StringView() = default;

    /**
     * @brief Construct a string view from a null-terminated (value 0) UTF-8 string
     *
     * @param utf8String UTF-8 string to view
     */
    constexpr StringView(const char8* utf8String);

    /**
     * @brief Construct a string view from an String
     *
     * @param string String to view
     */
    StringView(const String& string);

    /**
     * @brief Construct a string view from an StringView
     *
     * @param other StringView copy the view from
     */
    constexpr StringView(const StringView& other) = default;

    /**
     * @brief Destructor
     */
    ~StringView() = default;

    /**
     * @brief Overload of assignment operator
     *
     * @param right Instance to assign
     *
     * @return Reference to self
     */
    constexpr StringView& operator=(const StringView& right);

    /**
     * @brief Overload of assignment operator
     *
     * @param right Instance to assign
     *
     * @return Reference to self
     */
    StringView& operator=(const String& right);

    /**
     * @brief Overload of assignment operator
     *
     * @param right Instance to assign
     *
     * @return Reference to self
     */
    constexpr StringView& operator=(const char8* right);

    /**
     * @brief Overload of [] operator to access a character by its position
     *
     * This function provides read-only access to characters.
     * Note: the behavior is undefined if `index` is out of range.
     *
     * @param index Index of the character to get
     *
     * @return Character at position `index`
     */
    constexpr utf::CodeUnit<utf::UTF_8> operator[](size_type index) const;

    /**
     * @brief Get the size of the string
     *
     * @return Number of UTF-8 codepoints in the string
     *
     * @see isEmpty
     */
    constexpr size_type getSize() const;

    /**
     * @brief Check whether the string is empty or not
     *
     * @return True if the string is empty (i.e. contains no character)
     *
     * @see clear, getSize
     */
    constexpr bool isEmpty() const;

    /**
     * @brief Find a sequence of one or more characters in
     *        the string
     *
     * This function searches for the characters of `str`
     * in the string, starting from `start`.
     *
     * @param str   Characters to find
     * @param start Where to begin searching
     *
     * @return Position of `str` in the string, or @ref sInvalidPos
     *         if not found
     */
    constexpr size_type find(const StringView& str, size_type start = 0) const;

    /**
     * @brief Finds the first character equal to one of the
     *        characters in the given character sequence.
     *
     * This function searches for the first character equal to
     * one of the characters in `str`. The search considers
     * only the interval [`pos`, `getSize()`]
     *
     * @param str   Characters to find
     * @param pos Where to begin searching
     *
     * @return Position of the first character of `str` found
     *         in the string, or @ref sInvalidPos if not found
     */
    constexpr size_type findFirstOf(const StringView& str, size_type pos = 0) const;

    /**
     * @brief Finds the last character equal to one of the
     *        characters in the given character sequence.
     *
     * This function searches for the last character equal to
     * one of the characters in `str`. The search considers
     * only the interval [`0`, `pos`]
     *
     * @param str Characters to find
     * @param pos Where to begin searching
     *
     * @return Position of the last character of `str` found
     *        in the string, or @ref sInvalidPos if not found
     */
    constexpr size_type findLastOf(const StringView& str, size_type pos = sInvalidPos) const;

    /**
     * @brief Return a part of the string
     *
     * This function returns the SubString that starts at index `position`
     * and spans `length` characters.
     *
     * @param position Index of the first character
     * @param length   Number of characters to include in the SubString (if
     *                 the string is shorter, as many characters as possible
     *                 are included). @ref sInvalidPos can be used to include
     *                 all characters until the end of the string.
     *
     * @return String object containing a SubString of this object
     */
    constexpr StringView subString(size_type position, size_type length = sInvalidPos) const;

    /**
     * @brief Get a pointer to the C-style array of characters
     *
     * This functions provides a read-only access to a
     * null-terminated C-style representation of the string.
     * The returned pointer is temporary and is meant only for
     * immediate use, thus it is not recommended to store it.
     *
     * @return Read-only pointer to the array of characters
     */
    constexpr const char8* getData() const;

    /**
     * @brief Return an iterator to the beginning of the string
     *
     * @return Read-write iterator to the beginning of the string characters
     *
     * @see end
     */
    constexpr iterator begin();

    /**
     * @brief Return an iterator to the beginning of the string
     *
     * @return Read-only iterator to the beginning of the string characters
     *
     * @see end
     */
    constexpr const_iterator cbegin() const;

    /**
     * @brief Return an iterator to the end of the string
     *
     * This iterator refers to 1 position past the last character;
     * thus it represents an invalid character and should never be
     * accessed.
     *
     * @return Read-write iterator to the end of the string characters
     *
     * @see begin
     */
    constexpr iterator end();

    /**
     * @brief Return an iterator to the end of the string
     *
     * This iterator refers to 1 position past the last character;
     * thus it represents an invalid character and should never be
     * accessed.
     *
     * @return Read-only iterator to the end of the string characters
     *
     * @see begin
     */
    constexpr const_iterator cend() const;

    /**
     * @brief Return an iterator to the reverse beginning of the string
     *
     * This iterator refers to 1 position prior the first character;
     * thus it represents an invalid character and should never be
     * accessed.
     *
     * @return Read-only iterator to the reverse beginning of the string
     *
     * @see rend
     */
    reverse_iterator rbegin();

    /**
     * @brief Return an iterator to the reverse beginning of the string
     *
     * This iterator refers to 1 position prior the first character;
     * thus it represents an invalid character and should never be
     * accessed.
     *
     * @return Read-only iterator to the reverse beginning of the string
     *
     * @see crend
     */
    const_reverse_iterator crbegin() const;

    /**
     * @brief Return an iterator to the reverse end of the string
     *
     * This iterator refers to the first element in the string
     *
     * @return Read-only iterator to the end of the string
     *
     * @see rbegin
     */
    reverse_iterator rend();

    /**
     * @brief Return an iterator to the end of the string
     *
     * This iterator refers to the first element in the string
     *
     * @return Read-only iterator to the end of the string
     *
     * @see crbegin
     */
    const_reverse_iterator crend() const;

private:
    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    const char8* m_data = {nullptr};  ///< The begining of the string view
    size_type m_size = {0};           ///< The size of the string view
};

/**
 * @relates String
 * @brief Overload of == operator to compare two StringView
 *
 * @param left  Left operand (a StringView)
 * @param right Right operand (a StringView)
 *
 * @return True if both strings are equal
 */
ENGINE_API bool operator==(const StringView& left, const StringView& right);

/**
 * @relates String
 * @brief Overload of != operator to compare two StringView
 *
 * @param left  Left operand (a StringView)
 * @param right Right operand (a StringView)
 *
 * @return True if both strings are equal
 */
ENGINE_API bool operator!=(const StringView& left, const StringView& right);

/**
 * @relates String
 * @brief Overload of < operator to compare two StringView
 *
 * @param left  Left operand (a StringView)
 * @param right Right operand (a StringView)
 *
 * @return True if `left` is lexicographically before `right`
 */
ENGINE_API bool operator<(const StringView& left, const StringView& right);

/**
 * @relates String
 * @brief Overload of > operator to compare two StringView
 *
 * @param left  Left operand (a StringView)
 * @param right Right operand (a StringView)
 *
 * @return True if `left` is lexicographically after `right`
 */
ENGINE_API bool operator>(const StringView& left, const StringView& right);

/**
 * @relates String
 * @brief Overload of <= operator to compare two StringView
 *
 * @param left  Left operand (a StringView)
 * @param right Right operand (a StringView)
 *
 * @return True if `left` is lexicographically before or equivalent to \a
 *        right
 */
ENGINE_API bool operator<=(const StringView& left, const StringView& right);

/**
 * @relates String
 * @brief Overload of >= operator to compare two StringView
 *
 * @param left  Left operand (a StringView)
 * @param right Right operand (a StringView)
 *
 * @return True if `left` is lexicographically after or equivalent to `right`
 */
ENGINE_API bool operator>=(const StringView& left, const StringView& right);

/**
 * @relates String
 * @brief Overload of << operator add support to using the
 *        String class with STL ostream classes
 *
 * @param os  A std::ostream
 * @param str A String
 *
 * @return Returns os
 */
ENGINE_API std::ostream& operator<<(std::ostream& os, const StringView& str);

}  // namespace engine

#include <System/StringView.inl>
