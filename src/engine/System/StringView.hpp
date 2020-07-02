#pragma once

#include <Util/Prerequisites.hpp>

#include <System/String.hpp>
#include <Util/UTFIterator.hpp>

namespace engine {

/**
 * @brief Utility string class that automatically handles
 *       conversions between types and encodings
 */
class ENGINE_API StringView {
public:
    ////////////////////////////////////////////////////////////
    // Types
    ////////////////////////////////////////////////////////////
    using size_type = std::size_t;  ///< Size type
    // using char_type = char8;
    using const_iterator = UTFIterator<8, const char8*>;  ///< Read-only iterator type
    using iterator = const_iterator;                      ///< Iterator type

    ////////////////////////////////////////////////////////////
    // Static member data
    ////////////////////////////////////////////////////////////
    static const size_type sInvalidPos;  ///< Represents an invalid position in the string view

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
    constexpr StringView(const String& string);

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
    constexpr StringView& operator=(const String& right);

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
     * Note: the behavior is undefined if \a index is out of range.
     *
     * @param index Index of the character to get
     *
     * @return Character at position \a index
     */
    constexpr char8 operator[](std::size_t index) const;

    /**
     * @brief Get the size of the string
     *
     * @return Number of UTF-8 codepoints in the string
     *
     * @see IsEmpty
     */
    constexpr std::size_t getSize() const;

    /**
     * @brief Check whether the string is empty or not
     *
     * @return True if the string is empty (i.e. contains no character)
     *
     * @see Clear, getSize
     */
    constexpr bool isEmpty() const;

    /**
     * @brief Find a sequence of one or more characters in
     *        the string
     *
     * This function searches for the characters of \a str
     * in the string, starting from \a start.
     *
     * @param str   Characters to find
     * @param start Where to begin searching
     *
     * @return Position of \a str in the string, or String::InvalidPos
     *         if not found
     */
    constexpr std::size_t find(const StringView& str, std::size_t start = 0) const;

    /**
     * @brief Finds the first character equal to one of the
     *        characters in the given character sequence.
     *
     * This function searches for the first character equal to
     * one of the characters in \a str. The search considers
     * only the interval [\a pos, Size()]
     *
     * @param str   Characters to find
     * @param pos Where to begin searching
     *
     * @return Position of the first character of \a str found
     *        in the string, or String::InvalidPos if not found
     */
    constexpr std::size_t findFirstOf(const StringView& str, std::size_t pos = 0) const;

    /**
     * @brief Finds the last character equal to one of the
     *        characters in the given character sequence.
     *
     * This function searches for the last character equal to
     * one of the characters in \a str. The search considers
     * only the interval [0, \a pos]
     *
     * @param str Characters to find
     * @param pos Where to begin searching
     *
     * @return Position of the last character of \a str found
     *        in the string, or String::InvalidPos if not found
     */
    constexpr std::size_t findLastOf(const StringView& str, std::size_t pos = sInvalidPos) const;

    /**
     * @brief Return a part of the string
     *
     * This function returns the SubString that starts at index \a position
     * and spans \a length characters.
     *
     * @param position Index of the first character
     * @param length   Number of characters to include in the SubString (if
     *                 the string is shorter, as many characters as possible
     *                 are included). \ref InvalidPos can be used to include
     *                 all
     *                 characters until the end of the string.
     *
     * @return String object containing a SubString of this object
     */
    constexpr StringView subString(std::size_t position, std::size_t length = sInvalidPos) const;

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
    constexpr const_iterator begin();

    /**
     * @brief Return an iterator to the beginning of the string
     *
     * @return Read-only iterator to the beginning of the string characters
     *
     * @see end
     */
    constexpr const_iterator begin() const;

    /**
     * @brief Return an iterator to the end of the string
     *
     * The end iterator refers to 1 position past the last character;
     * thus it represents an invalid character and should never be
     * accessed.
     *
     * @return Read-write iterator to the end of the string characters
     *
     * @see begin
     */
    constexpr const_iterator end();

    /**
     * @brief Return an iterator to the end of the string
     *
     * The end iterator refers to 1 position past the last character;
     * thus it represents an invalid character and should never be
     * accessed.
     *
     * @return Read-only iterator to the end of the string characters
     *
     * @see begin
     */
    constexpr const_iterator end() const;

private:
    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    const char8* m_data = {nullptr};  ///< The begining of the string view
    size_t m_size = {0};              ///< The size of the string view
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
 * @return True if \a left is lexicographically before \a right
 */
ENGINE_API bool operator<(const StringView& left, const StringView& right);

/**
 * @relates String
 * @brief Overload of > operator to compare two StringView
 *
 * @param left  Left operand (a StringView)
 * @param right Right operand (a StringView)
 *
 * @return True if \a left is lexicographically after \a right
 */
ENGINE_API bool operator>(const StringView& left, const StringView& right);

/**
 * @relates String
 * @brief Overload of <= operator to compare two StringView
 *
 * @param left  Left operand (a StringView)
 * @param right Right operand (a StringView)
 *
 * @return True if \a left is lexicographically before or equivalent to \a
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
 * @return True if \a left is lexicographically after or equivalent to \a right
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

/**
 * @class String
 *
 * String is a utility string class defined mainly for
 * convenience. It is a Unicode string (implemented using
 * UTF-8), thus it can store any character in the world
 * (European, Chinese, Arabic, Hebrew, etc.).
 *
 * It automatically handles conversions from/to ASCII and
 * wide strings, so that you can work with standard string
 * classes and still be compatible with functions taking a
 * String.
 *
 * @code
 * String s;
 *
 * std::basic_string<char8> s1 = s;  // automatically converted to ASCII string
 * std::basic_string<wchar> s2 = s; // automatically converted to wide string
 * s = "hello";         // automatically converted from ASCII string
 * s = L"hello";        // automatically converted from wide string
 * s += 'a';            // automatically converted from ASCII string
 * s += L'a';           // automatically converted from wide string
 * @endcode
 *
 * String defines the most important functions of the
 * standard std::basic_string<char8> class: removing, random access, iterating,
 * appending, comparing, etc.
 */

#include <System/StringView.inl>
