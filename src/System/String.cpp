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

#include <System/String.hpp>

#include <utf8.h>

namespace engine {

const std::size_t String::InvalidPos = std::basic_string<char8>::npos;

String::String() {}

String::String(char8 asciiChar) {
    m_string += asciiChar;
}

String::String(char16 utf16Char) {
    char16* ptr = &utf16Char;
    utf8::utf16to8(ptr, ptr + 1, std::back_inserter(m_string));
}

String::String(char32 utf32Char) {
    char32* ptr = &utf32Char;
    utf8::utf32to8(ptr, ptr + 1, std::back_inserter(m_string));
}

String::String(const char* utf8String) {
    if (utf8String) {
        std::size_t length = strlen(utf8String);
        if (length > 0) {
            if (utf8::is_valid(utf8String, utf8String + length)) {
                m_string.assign(utf8String);
            } else {
                throw std::runtime_error("invalid utf8 convertion.");
            }
        };
    }
}

String::String(const std::basic_string<char8>& utf8String) {
    if (utf8String.size() > 0) {
        if (utf8::is_valid(utf8String.cbegin(), utf8String.cend())) {
            m_string.assign(utf8String);
        } else {
            throw std::runtime_error("invalid utf8 convertion.");
        }
    };
}

String::String(const std::basic_string<char16>& utf16String) {
    utf8::utf16to8(utf16String.cbegin(), utf16String.cend(),
                   std::back_inserter(m_string));
}

String::String(const std::basic_string<char32>& utf32String) {
    utf8::utf32to8(utf32String.cbegin(), utf32String.cend(),
                   std::back_inserter(m_string));
}

String::String(const String& other) : m_string(other.m_string) {}

String::String(String&& other) : m_string(std::move(other.m_string)) {}

String String::FromUtf8(const char8* begin, const char8* end) {
    String string;
    if (utf8::is_valid(begin, end)) {
        string.m_string.assign(begin, end);
    } else {
        throw std::runtime_error("invalid utf8 convertion.");
    }
    return string;
}

String String::FromUtf16(const char16* begin, const char16* end) {
    String string;
    utf8::utf16to8(begin, end, std::back_inserter(string.m_string));
    return string;
}

String String::FromUtf32(const char32* begin, const char32* end) {
    String string;
    utf8::utf32to8(begin, end, std::back_inserter(string.m_string));
    return string;
}

String::operator std::basic_string<char8>() const {
    return ToUtf8();
}

String::operator std::basic_string<char16>() const {
    return ToUtf16();
}

String::operator std::basic_string<char32>() const {
    return ToUtf32();
}

const std::basic_string<char8>& String::ToUtf8() const {
    return m_string;
}

std::basic_string<char16> String::ToUtf16() const {
    std::basic_string<char16> output;
    utf8::utf8to16(m_string.cbegin(), m_string.cend(),
                   std::back_inserter(output));
    return output;
}

std::basic_string<char32> String::ToUtf32() const {
    std::basic_string<char32> output;
    utf8::utf8to32(m_string.cbegin(), m_string.cend(),
                   std::back_inserter(output));
    return output;
}

String& String::operator=(const String& right) {
    m_string = right.m_string;
    return *this;
}

String& String::operator=(String&& right) {
    m_string = std::move(right.m_string);
    return *this;
}

String& String::operator+=(const String& right) {
    m_string += right.m_string;
    return *this;
}

char String::operator[](std::size_t index) const {
    return m_string[index];
}

char& String::operator[](std::size_t index) {
    return m_string[index];
}

void String::Clear() {
    m_string.clear();
}

std::size_t String::GetSize() const {
    return utf8::distance(m_string.begin(), m_string.end());
}

bool String::IsEmpty() const {
    return m_string.empty();
}

void String::Erase(std::size_t position, std::size_t count) {
    // Iterate to the start codepoint
    auto start_it(m_string.begin());
    for (std::size_t i = 0; i < position; i++) {
        try {
            utf8::next(start_it, m_string.end());
        } catch (utf8::not_enough_room) {
            throw std::out_of_range(
                "the specified position is out of the string range");
        }
    }
    // Iterate to the end codepoint
    auto end_it(start_it);
    for (std::size_t i = 0; i < count; i++) {
        utf8::next(end_it, m_string.end());
        if (end_it == m_string.end()) break;
    }
    m_string.erase(start_it, end_it);
}

void String::Insert(std::size_t position, const String& str) {
    // Iterate to the start codepoint
    auto start_it(m_string.begin());
    for (std::size_t i = 0; i < position; i++) {
        try {
            utf8::next(start_it, m_string.end());
        } catch (utf8::not_enough_room) {
            throw std::out_of_range(
                "the specified position is out of the string range");
        }
    }
    m_string.insert(start_it, str.m_string.cbegin(), str.m_string.cend());
}

std::size_t String::Find(const String& str, std::size_t start) const {
    // Iterate to the start codepoint
    auto start_it(m_string.cbegin());
    for (std::size_t i = 0; i < start; i++) {
        utf8::next(start_it, m_string.cend());
        if (start_it == m_string.end()) return InvalidPos;
    }
    // Find the string
    auto find_it(std::search(start_it, m_string.cend(), str.m_string.cbegin(),
                             str.m_string.cend()));
    return (find_it == m_string.cend())
               ? InvalidPos
               : utf8::distance(m_string.cbegin(), find_it);
}

void String::Replace(std::size_t position, std::size_t length,
                     const String& replaceWith) {
    // Iterate to the start codepoint
    auto start_it(m_string.begin());
    for (std::size_t i = 0; i < position; i++) {
        try {
            utf8::next(start_it, m_string.end());
        } catch (utf8::not_enough_room) {
            throw std::out_of_range(
                "the specified position is out of the string range");
        }
    }
    // Iterate to the end codepoint
    auto end_it(start_it);
    for (std::size_t i = 0; i < length; i++) {
        utf8::next(end_it, m_string.end());
        if (end_it == m_string.end()) break;
    }
    m_string.replace(start_it, end_it, replaceWith.m_string);
}

void String::Replace(const String& searchFor, const String& replaceWith) {
    std::size_t step = replaceWith.GetSize();
    std::size_t len = searchFor.GetSize();
    std::size_t pos = Find(searchFor);

    // Replace each occurrence of search
    while (pos != InvalidPos) {
        Replace(pos, len, replaceWith);
        pos = Find(searchFor, pos + step);
    }
}

String String::SubString(std::size_t position, std::size_t length) const {
    // Iterate to the start codepoint
    auto start_it(m_string.begin());
    for (std::size_t i = 0; i < position; i++) {
        try {
            utf8::next(start_it, m_string.end());
        } catch (utf8::not_enough_room) {
            throw std::out_of_range(
                "the specified position is out of the string range");
        }
    }
    // Iterate to the end codepoint
    auto end_it(start_it);
    for (std::size_t i = 0; i < length; i++) {
        utf8::next(end_it, m_string.end());
        if (end_it == m_string.end()) break;
    }
    return String::FromUtf8(&(*start_it), &(*end_it));
}

const char8* String::GetData() const {
    return m_string.data();
}

/////////////////////////// TODO

String::iterator String::Begin() {
    return m_string.begin();
}

String::const_iterator String::Begin() const {
    return m_string.begin();
}

String::iterator String::End() {
    return m_string.end();
}

String::const_iterator String::End() const {
    return m_string.end();
}

/////////////////////////// DOTO

bool operator==(const String& left, const String& right) {
    return left.m_string == right.m_string;
}

bool operator!=(const String& left, const String& right) {
    return !(left == right);
}

bool operator<(const String& left, const String& right) {
    return left.m_string < right.m_string;
}

bool operator>(const String& left, const String& right) {
    return right < left;
}

bool operator<=(const String& left, const String& right) {
    return !(right < left);
}

bool operator>=(const String& left, const String& right) {
    return !(left < right);
}

String operator+(const String& left, const String& right) {
    String string = left;
    string += right;

    return string;
}

}  // namespace engine
