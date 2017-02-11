////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2016 Laurent Gomila (laurent@sfml-dev.org)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the
// use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

#pragma once

#include <Util/Prerequisites.hpp>

namespace engine {
////////////////////////////////////////////////////////////
/// \brief Utility string class that automatically handles
///        conversions between types and encodings
///
////////////////////////////////////////////////////////////
class ENGINE_API String {
public:
    ////////////////////////////////////////////////////////////
    // Types
    ////////////////////////////////////////////////////////////
    typedef std::string::iterator iterator;  ///< Iterator type
    typedef std::string::const_iterator
        const_iterator;  ///< Read-only iterator type

    ////////////////////////////////////////////////////////////
    // Static member data
    ////////////////////////////////////////////////////////////
    static const std::size_t
        InvalidPos;  ///< Represents an invalid position in the string

    ////////////////////////////////////////////////////////////
    /// \brief Default constructor
    ///
    /// This constructor creates an empty string.
    ///
    ////////////////////////////////////////////////////////////
    String();

    ////////////////////////////////////////////////////////////
    /// \brief Construct from a single ASCII character
    ///
    /// \param asciiChar ASCII character to convert
    ///
    ////////////////////////////////////////////////////////////
    String(char asciiChar);

    ////////////////////////////////////////////////////////////
    /// \brief Construct from single UTF-16 character
    ///
    /// \param utf32Char UTF-16 character to convert
    ///
    ////////////////////////////////////////////////////////////
    String(char16_t utf16Char);

    ////////////////////////////////////////////////////////////
    /// \brief Construct from single UTF-32 character
    ///
    /// \param utf32Char UTF-32 character to convert
    ///
    ////////////////////////////////////////////////////////////
    String(char32_t utf32Char);

    ////////////////////////////////////////////////////////////
    /// \brief Construct from a null-terminated UTF-8 string
    ///
    /// \param utf8String ASCII string to convert
    ///
    ////////////////////////////////////////////////////////////
    String(const char* utf8String);

    ////////////////////////////////////////////////////////////
    /// \brief Construct from an UTF-8 string
    ///
    /// \param utf8String ASCII string to convert
    ///
    ////////////////////////////////////////////////////////////
    String(const std::string& utf8String);

    ////////////////////////////////////////////////////////////
    /// \brief Construct from a UTF-16 string
    ///
    /// \param utf16String Wide string to convert
    ///
    ////////////////////////////////////////////////////////////
    String(const std::u16string& utf16String);

    ////////////////////////////////////////////////////////////
    /// \brief Construct from an UTF-32 string
    ///
    /// \param utf32String UTF-32 string to assign
    ///
    ////////////////////////////////////////////////////////////
    String(const std::u32string& utf32String);

    ////////////////////////////////////////////////////////////
    /// \brief Copy constructor
    ///
    /// \param other Instance to copy
    ///
    ////////////////////////////////////////////////////////////
    String(const String& other);

    ////////////////////////////////////////////////////////////
    /// \brief Move constructor
    ///
    /// \param other Instance to move
    ///
    ////////////////////////////////////////////////////////////
    String(String&& other);

    ////////////////////////////////////////////////////////////
    /// \brief Create a new String from a UTF-8 encoded string
    ///
    /// This function is provided for consistency, it is equivalent to
    /// using the constructors that takes a const char* or a std::string.
    ///
    /// \param begin Forward iterator to the beginning of the UTF-8 sequence
    /// \param end   Forward iterator to the end of the UTF-8 sequence
    ///
    /// \return A String containing the source string
    ///
    /// \see FromUtf16, FromUtf32
    ///
    ////////////////////////////////////////////////////////////
    template <typename T>
    static String FromUtf8(T begin, T end) {
        String string;
        if (utf8::is_valid(begin, end)) {
            string.string_.assign(begin, end);
        } else {
            throw std::exception("invalid utf8 convertion.");
        }
        return string;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Create a new String from a UTF-16 encoded string
    ///
    /// \param begin Forward iterator to the beginning of the UTF-16 sequence
    /// \param end   Forward iterator to the end of the UTF-16 sequence
    ///
    /// \return A String containing the source string
    ///
    /// \see FromUtf8, FromUtf32
    ///
    ////////////////////////////////////////////////////////////
    template <typename T>
    static String FromUtf16(T begin, T end) {
        String string;
        utf8::utf16to8(begin, end, std::back_inserter(string.string_));
        return string;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Create a new String from a UTF-32 encoded string
    ///
    /// \param begin Forward iterator to the beginning of the UTF-32 sequence
    /// \param end   Forward iterator to the end of the UTF-32 sequence
    ///
    /// \return A String containing the source string
    ///
    /// \see FromUtf8, FromUtf16
    ///
    ////////////////////////////////////////////////////////////
    template <typename T>
    static String FromUtf32(T begin, T end) {
        String string;
        utf8::utf32to8(begin, end, std::back_inserter(string.string_));
        return string;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Implicit conversion operator to std::string (UTF-8 string)
    ///
    /// \return Converted UTF-8 string
    ///
    /// \see ToUtf8, operator std::u16string, operator std::u32string
    ///
    ////////////////////////////////////////////////////////////
    operator std::string() const;

    ////////////////////////////////////////////////////////////
    /// \brief Implicit conversion operator to std::u16string (UTF-16 string)
    ///
    /// \return Converted UTF-16 string
    ///
    /// \see ToUtf16, operator std::string, operator std::u32string
    ///
    ////////////////////////////////////////////////////////////
    operator std::u16string() const;

    ////////////////////////////////////////////////////////////
    /// \brief Implicit conversion operator to std::u32string (UTF-32 string)
    ///
    /// \return Converted UTF-32 string
    ///
    /// \see ToUtf32,, operator std::string, operator std::u16string
    ///
    ////////////////////////////////////////////////////////////
    operator std::u32string() const;

    ////////////////////////////////////////////////////////////
    /// \brief Convert the Unicode string to a UTF-8 string
    ///
    /// This function doesn't perform any conversion, since the
    /// string is already stored as UTF-8 internally.
    ///
    /// \return Converted UTF-8 string
    ///
    /// \see ToUtf16, ToUtf32
    ///
    ////////////////////////////////////////////////////////////
    const std::string& ToUtf8() const;

    ////////////////////////////////////////////////////////////
    /// \brief Convert the Unicode string to a UTF-16 string
    ///
    /// \return Converted UTF-16 string
    ///
    /// \see ToUtf8, ToUtf32
    ///
    ////////////////////////////////////////////////////////////
    std::u16string ToUtf16() const;

    ////////////////////////////////////////////////////////////
    /// \brief Convert the Unicode string to a UTF-32 string
    ///
    /// \return Converted UTF-32 string
    ///
    /// \see ToUtf8, ToUtf16
    ///
    ////////////////////////////////////////////////////////////
    std::u32string ToUtf32() const;

    ////////////////////////////////////////////////////////////
    /// \brief Overload of assignment operator
    ///
    /// \param right Instance to assign
    ///
    /// \return Reference to self
    ///
    ////////////////////////////////////////////////////////////
    String& operator=(const String& right);

    ////////////////////////////////////////////////////////////
    /// \brief Overload of move assignment operator
    ///
    /// \param right Instance to move
    ///
    /// \return Reference to self
    ///
    ////////////////////////////////////////////////////////////
    String& operator=(String&& right);

    ////////////////////////////////////////////////////////////
    /// \brief Overload of += operator to append an UTF-8 string
    ///
    /// \param right String to append
    ///
    /// \return Reference to self
    ///
    ////////////////////////////////////////////////////////////
    String& operator+=(const String& right);

    ////////////////////////////////////////////////////////////
    /// \brief Overload of [] operator to access a character by its position
    ///
    /// This function provides read-only access to characters.
    /// Note: the behavior is undefined if \a index is out of range.
    ///
    /// \param index Index of the character to get
    ///
    /// \return Character at position \a index
    ///
    ////////////////////////////////////////////////////////////
    char operator[](std::size_t index) const;

    ////////////////////////////////////////////////////////////
    /// \brief Overload of [] operator to access a character by its position
    ///
    /// This function provides read and write access to characters.
    /// Note: the behavior is undefined if \a index is out of range.
    ///
    /// \param index Index of the character to get
    ///
    /// \return Reference to the character at position \a index
    ///
    ////////////////////////////////////////////////////////////
    char& operator[](std::size_t index);

    ////////////////////////////////////////////////////////////
    /// \brief Clear the string
    ///
    /// This function removes all the characters from the string.
    ///
    /// \see isEmpty, erase
    ///
    ////////////////////////////////////////////////////////////
    void Clear();

    ////////////////////////////////////////////////////////////
    /// \brief Get the size of the string
    ///
    /// \return Number of characters in the string
    ///
    /// \see isEmpty
    ///
    ////////////////////////////////////////////////////////////
    std::size_t GetSize() const;

    ////////////////////////////////////////////////////////////
    /// \brief Check whether the string is empty or not
    ///
    /// \return True if the string is empty (i.e. contains no character)
    ///
    /// \see Clear, getSize
    ///
    ////////////////////////////////////////////////////////////
    bool IsEmpty() const;

    ////////////////////////////////////////////////////////////
    /// \brief Erase one or more characters from the string
    ///
    /// This function removes a sequence of \a count characters
    /// starting from \a position.
    ///
    /// \param position Position of the first character to erase
    /// \param count    Number of characters to erase
    ///
    ////////////////////////////////////////////////////////////
    void Erase(std::size_t position, std::size_t count = 1);

    ////////////////////////////////////////////////////////////
    /// \brief Insert one or more characters into the string
    ///
    /// This function inserts the characters of \a str
    /// into the string, starting from \a position.
    ///
    /// \param position Position of insertion
    /// \param str      Characters to insert
    ///
    ////////////////////////////////////////////////////////////
    void Insert(std::size_t position, const String& str);

    ////////////////////////////////////////////////////////////
    /// \brief Find a sequence of one or more characters in the string
    ///
    /// This function searches for the characters of \a str
    /// in the string, starting from \a start.
    ///
    /// \param str   Characters to find
    /// \param start Where to begin searching
    ///
    /// \return Position of \a str in the string, or String::InvalidPos if not
    /// found
    ///
    ////////////////////////////////////////////////////////////
    std::size_t Find(const String& str, std::size_t start = 0) const;

    ////////////////////////////////////////////////////////////
    /// \brief Replace a SubString with another string
    ///
    /// This function replaces the SubString that starts at index \a position
    /// and spans \a length characters with the string \a replaceWith.
    ///
    /// \param position    Index of the first character to be replaced
    /// \param length      Number of characters to replace. You can pass
    /// InvalidPos to
    ///                    replace all characters until the end of the string.
    /// \param replaceWith String that replaces the given SubString.
    ///
    ////////////////////////////////////////////////////////////
    void Replace(std::size_t position, std::size_t length,
                 const String& replaceWith);

    ////////////////////////////////////////////////////////////
    /// \brief Replace all occurrences of a SubString with a replacement string
    ///
    /// This function replaces all occurrences of \a searchFor in this string
    /// with the string \a replaceWith.
    ///
    /// \param searchFor   The value being searched for
    /// \param replaceWith The value that replaces found \a searchFor values
    ///
    ////////////////////////////////////////////////////////////
    void Replace(const String& searchFor, const String& replaceWith);

    ////////////////////////////////////////////////////////////
    /// \brief Return a part of the string
    ///
    /// This function returns the SubString that starts at index \a position
    /// and spans \a length characters.
    ///
    /// \param position Index of the first character
    /// \param length   Number of characters to include in the SubString (if
    ///                 the string is shorter, as many characters as possible
    ///                 are included). \ref InvalidPos can be used to include
    ///                 all
    ///                 characters until the end of the string.
    ///
    /// \return String object containing a SubString of this object
    ///
    ////////////////////////////////////////////////////////////
    String SubString(std::size_t position,
                     std::size_t length = InvalidPos) const;

    ////////////////////////////////////////////////////////////
    /// \brief Get a pointer to the C-style array of characters
    ///
    /// This functions provides a read-only access to a
    /// null-terminated C-style representation of the string.
    /// The returned pointer is temporary and is meant only for
    /// immediate use, thus it is not recommended to store it.
    ///
    /// \return Read-only pointer to the array of characters
    ///
    ////////////////////////////////////////////////////////////
    const std::string::value_type* GetData() const;

    ////////////////////////////////////////////////////////////
    /// \brief Return an iterator to the beginning of the string
    ///
    /// \return Read-write iterator to the beginning of the string characters
    ///
    /// \see end
    ///
    ////////////////////////////////////////////////////////////
    iterator Begin();

    ////////////////////////////////////////////////////////////
    /// \brief Return an iterator to the beginning of the string
    ///
    /// \return Read-only iterator to the beginning of the string characters
    ///
    /// \see end
    ///
    ////////////////////////////////////////////////////////////
    const_iterator Begin() const;

    ////////////////////////////////////////////////////////////
    /// \brief Return an iterator to the end of the string
    ///
    /// The end iterator refers to 1 position past the last character;
    /// thus it represents an invalid character and should never be
    /// accessed.
    ///
    /// \return Read-write iterator to the end of the string characters
    ///
    /// \see begin
    ///
    ////////////////////////////////////////////////////////////
    iterator End();

    ////////////////////////////////////////////////////////////
    /// \brief Return an iterator to the end of the string
    ///
    /// The end iterator refers to 1 position past the last character;
    /// thus it represents an invalid character and should never be
    /// accessed.
    ///
    /// \return Read-only iterator to the end of the string characters
    ///
    /// \see begin
    ///
    ////////////////////////////////////////////////////////////
    const_iterator End() const;

private:
    friend ENGINE_API bool operator==(const String& left,
                                                 const String& right);
    friend ENGINE_API bool operator<(const String& left,
                                                const String& right);

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    std::string string_;  ///< Internal string of UTF-8 characters
};

////////////////////////////////////////////////////////////
/// \relates String
/// \brief Overload of == operator to compare two UTF-8 strings
///
/// \param left  Left operand (a string)
/// \param right Right operand (a string)
///
/// \return True if both strings are equal
///
////////////////////////////////////////////////////////////
ENGINE_API bool operator==(const String& left, const String& right);

////////////////////////////////////////////////////////////
/// \relates String
/// \brief Overload of != operator to compare two UTF-8 strings
///
/// \param left  Left operand (a string)
/// \param right Right operand (a string)
///
/// \return True if both strings are different
///
////////////////////////////////////////////////////////////
ENGINE_API bool operator!=(const String& left, const String& right);

////////////////////////////////////////////////////////////
/// \relates String
/// \brief Overload of < operator to compare two UTF-8 strings
///
/// \param left  Left operand (a string)
/// \param right Right operand (a string)
///
/// \return True if \a left is lexicographically before \a right
///
////////////////////////////////////////////////////////////
ENGINE_API bool operator<(const String& left, const String& right);

////////////////////////////////////////////////////////////
/// \relates String
/// \brief Overload of > operator to compare two UTF-8 strings
///
/// \param left  Left operand (a string)
/// \param right Right operand (a string)
///
/// \return True if \a left is lexicographically after \a right
///
////////////////////////////////////////////////////////////
ENGINE_API bool operator>(const String& left, const String& right);

////////////////////////////////////////////////////////////
/// \relates String
/// \brief Overload of <= operator to compare two UTF-8 strings
///
/// \param left  Left operand (a string)
/// \param right Right operand (a string)
///
/// \return True if \a left is lexicographically before or equivalent to \a
/// right
///
////////////////////////////////////////////////////////////
ENGINE_API bool operator<=(const String& left, const String& right);

////////////////////////////////////////////////////////////
/// \relates String
/// \brief Overload of >= operator to compare two UTF-8 strings
///
/// \param left  Left operand (a string)
/// \param right Right operand (a string)
///
/// \return True if \a left is lexicographically after or equivalent to \a right
///
////////////////////////////////////////////////////////////
ENGINE_API bool operator>=(const String& left, const String& right);

////////////////////////////////////////////////////////////
/// \relates String
/// \brief Overload of binary + operator to concatenate two strings
///
/// \param left  Left operand (a string)
/// \param right Right operand (a string)
///
/// \return Concatenated string
///
////////////////////////////////////////////////////////////
ENGINE_API String operator+(const String& left, const String& right);

}  // namespace engine

////////////////////////////////////////////////////////////
/// \class String
/// \ingroup system
///
/// String is a utility string class defined mainly for
/// convenience. It is a Unicode string (implemented using
/// UTF-8), thus it can store any character in the world
/// (European, Chinese, Arabic, Hebrew, etc.).
///
/// It automatically handles conversions from/to ASCII and
/// wide strings, so that you can work with standard string
/// classes and still be compatible with functions taking a
/// String.
///
/// \code
/// String s;
///
/// std::string s1 = s;  // automatically converted to ASCII string
/// std::wstring s2 = s; // automatically converted to wide string
/// s = "hello";         // automatically converted from ASCII string
/// s = L"hello";        // automatically converted from wide string
/// s += 'a';            // automatically converted from ASCII string
/// s += L'a';           // automatically converted from wide string
/// \endcode
///
/// String defines the most important functions of the
/// standard std::string class: removing, random access, iterating,
/// appending, comparing, etc.
///
////////////////////////////////////////////////////////////
