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

const std::size_t String::sInvalidPos = std::basic_string<char8>::npos;

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

String::String(const char8* utf8String) {
    if (utf8String && utf8String[0] != 0) {
        std::size_t length = std::strlen(utf8String);
        if (length > 0) {
            if (utf8::is_valid(utf8String, utf8String + length)) {
                m_string.assign(utf8String);
            } else {
                ENGINE_THROW(std::runtime_error("invalid utf8 convertion."));
            }
        };
    }
}

String::String(const char16* utf16String) {
    if (utf16String && utf16String[0] != 0) {
        // Find the lenght
        const char16* it = utf16String;
        while (*(++it) != 0) {
        }
        std::size_t length = it - utf16String;
        utf8::utf16to8(utf16String, utf16String + length, std::back_inserter(m_string));
    }
}

String::String(const char32* utf32String) {
    if (utf32String && utf32String[0] != 0) {
        // Find the lenght
        const char32* it = utf32String;
        while (*(++it) != 0) {
        }
        std::size_t length = it - utf32String;
        utf8::utf32to8(utf32String, utf32String + length, std::back_inserter(m_string));
    }
}

String::String(const wchar* wideString) {
    if (wideString && wideString[0] != 0) {
        std::size_t length = std::wcslen(wideString);
#if PLATFORM_IS(PLATFORM_WINDOWS)
        utf8::utf16to8(wideString, wideString + length, std::back_inserter(m_string));
#else
        utf8::utf32to8(wideString, wideString + length, std::back_inserter(m_string));
#endif
    }
}

String::String(const std::basic_string<char8>& utf8String) {
    if (!utf8String.empty()) {
        if (utf8::is_valid(utf8String.cbegin(), utf8String.cend())) {
            m_string.assign(utf8String);
        } else {
            ENGINE_THROW(std::runtime_error("invalid utf8 convertion."));
        }
    };
}

String::String(std::basic_string<char8>&& utf8String) {
    if (!utf8String.empty()) {
        if (utf8::is_valid(utf8String.cbegin(), utf8String.cend())) {
            m_string = std::move(utf8String);
        } else {
            ENGINE_THROW(std::runtime_error("invalid utf8 convertion."));
        }
    };
}

String::String(const std::basic_string<char16>& utf16String) {
    utf8::utf16to8(utf16String.cbegin(), utf16String.cend(), std::back_inserter(m_string));
}

String::String(const std::basic_string<char32>& utf32String) {
    utf8::utf32to8(utf32String.cbegin(), utf32String.cend(), std::back_inserter(m_string));
}

String::String(const std::basic_string<wchar>& wideString) {
    utf8::utf32to8(wideString.cbegin(), wideString.cend(), std::back_inserter(m_string));
}

String::String(const String& other) : m_string(other.m_string) {}

String::String(String&& other) : m_string(std::move(other.m_string)) {}

String::~String() {}

String String::FromUtf8(const char8* begin, const char8* end) {
    String string;
    if (utf8::is_valid(begin, end)) {
        string.m_string.assign(begin, end);
    } else {
        ENGINE_THROW(std::runtime_error("invalid utf8 convertion."));
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

String String::FromWide(const wchar* begin, const wchar* end) {
#if PLATFORM_IS(PLATFORM_WINDOWS)
    return FromUtf16(reinterpret_cast<const char16*>(begin), reinterpret_cast<const char16*>(end));
#else
    return FromUtf32(reinterpret_cast<const char32*>(begin), reinterpret_cast<const char32*>(end));
#endif
}

String::operator const char*() const {
    return toUtf8().data();
}

String::operator std::basic_string<char8>() const {
    return toUtf8();
}

String::operator std::basic_string<char16>() const {
    return toUtf16();
}

String::operator std::basic_string<char32>() const {
    return toUtf32();
}

String::operator std::basic_string<wchar>() const {
    return toWide();
}

const std::basic_string<char8>& String::toUtf8() const {
    return m_string;
}

std::basic_string<char16> String::toUtf16() const {
    std::basic_string<char16> output;
    utf8::utf8to16(m_string.cbegin(), m_string.cend(), std::back_inserter(output));
    return output;
}

std::basic_string<char32> String::toUtf32() const {
    std::basic_string<char32> output;
    utf8::utf8to32(m_string.cbegin(), m_string.cend(), std::back_inserter(output));
    return output;
}

std::basic_string<wchar> String::toWide() const {
    std::basic_string<wchar> output;
#if PLATFORM_IS(PLATFORM_WINDOWS)
    utf8::utf8to16(m_string.cbegin(), m_string.cend(), std::back_inserter(output));
#else
    utf8::utf8to32(m_string.cbegin(), m_string.cend(), std::back_inserter(output));
#endif
    return output;
}

String& String::operator=(const String& right) {
    m_string = right.m_string;
    return *this;
}

String& String::operator=(const char8* right) {
    m_string = right;
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

String& String::operator+=(const char8* right) {
    m_string += right;
    return *this;
}

String& String::operator+=(char8 right) {
    if (right >= 0) {
        m_string += right;
    }
    return *this;
}

char8 String::operator[](std::size_t index) const {
    return m_string[index];
}

char8& String::operator[](std::size_t index) {
    return m_string[index];
}

void String::clear() {
    m_string.clear();
}

std::size_t String::getSize() const {
    return utf8::distance(m_string.begin(), m_string.end());
}

bool String::isEmpty() const {
    return m_string.empty();
}

void String::erase(std::size_t position, std::size_t count) {
    // Iterate to the start codepoint
    auto start_it(m_string.begin());
    for (std::size_t i = 0; i < position; i++) {
        ENGINE_TRY {
            utf8::next(start_it, m_string.end());
        }
        ENGINE_CATCH(utf8::not_enough_room) {
            ENGINE_THROW(std::out_of_range("the specified position is out of the string range"));
        }
    }
    // Iterate to the end codepoint
    auto end_it(start_it);
    for (std::size_t i = 0; i < count; i++) {
        utf8::next(end_it, m_string.end());
        if (end_it == m_string.end()) {
            break;
        }
    }
    m_string.erase(start_it, end_it);
}

void String::insert(std::size_t position, const String& str) {
    // Iterate to the start codepoint
    auto start_it(m_string.begin());
    for (std::size_t i = 0; i < position; i++) {
        ENGINE_TRY {
            utf8::next(start_it, m_string.end());
        }
        ENGINE_CATCH(utf8::not_enough_room) {
            ENGINE_THROW(std::out_of_range("the specified position is out of the string range"));
        }
    }
    m_string.insert(start_it, str.m_string.cbegin(), str.m_string.cend());
}

std::size_t String::find(const String& str, std::size_t start) const {
    // Iterate to the start codepoint
    auto start_it(m_string.cbegin());
    for (std::size_t i = 0; i < start; i++) {
        utf8::next(start_it, m_string.cend());
        if (start_it == m_string.cend()) {
            return sInvalidPos;
        }
    }
    // Find the string
    auto find_it(std::search(start_it, m_string.cend(), str.m_string.cbegin(), str.m_string.cend()));
    return (find_it == m_string.cend()) ? sInvalidPos : utf8::distance(m_string.cbegin(), find_it);
}

std::size_t String::findFirstOf(const String& str, std::size_t pos) const {
    size_t str_size = getSize();

    if (pos >= str_size) {
        return sInvalidPos;
    }

    // Iterate to the start codepoint
    auto start_it(m_string.cbegin());
    for (std::size_t i = 0; i < pos; i++) {
        utf8::next(start_it, m_string.cend());
        if (start_it == m_string.cend()) {
            return sInvalidPos;
        }
    }

    // Find one of the UTF-8 codepoints
    auto end_it(start_it);
    while (true) {
        if (start_it == m_string.cend()) {
            return sInvalidPos;
        }
        utf8::next(end_it, m_string.cend());
        auto find_it(std::search(str.m_string.cbegin(), str.m_string.cend(), start_it, end_it));
        if (find_it != str.m_string.cend()) {
            return utf8::distance(m_string.cbegin(), start_it);
        }
        start_it = end_it;
    }
}

std::size_t String::findLastOf(const String& str, std::size_t pos) const {
    // Iterate to the start codepoint
    auto start_it(m_string.cbegin());
    if (pos == sInvalidPos) {
        start_it = m_string.cend();
    } else {
        for (std::size_t i = 0; i < pos + 1; i++) {
            utf8::next(start_it, m_string.cend());
            if (start_it == m_string.cend()) {
                break;
            }
        }
    }

    // Find one of the UTF-8 codepoints
    auto end_it(start_it);
    while (true) {
        if (start_it == m_string.cbegin()) {
            return sInvalidPos;
        }
        utf8::prior(end_it, m_string.cbegin());
        auto find_it(std::search(str.m_string.cbegin(), str.m_string.cend(), end_it, start_it));
        if (find_it != str.m_string.cend()) {
            return utf8::distance(m_string.cbegin(), end_it);
        }
        start_it = end_it;
    }

    return sInvalidPos;
}

void String::replace(std::size_t position, std::size_t length, const String& replaceWith) {
    // Iterate to the start codepoint
    auto start_it(m_string.begin());
    for (std::size_t i = 0; i < position; i++) {
        ENGINE_TRY {
            utf8::next(start_it, m_string.end());
        }
        ENGINE_CATCH(utf8::not_enough_room) {
            ENGINE_THROW(std::out_of_range("the specified position is out of the string range"));
        }
    }
    // Iterate to the end codepoint
    auto end_it(start_it);
    for (std::size_t i = 0; i < length; i++) {
        utf8::next(end_it, m_string.end());
        if (end_it == m_string.end()) {
            break;
        }
    }
    m_string.replace(start_it, end_it, replaceWith.m_string);
}

void String::replace(uint32 searchFor, uint32 replaceWith) {
    if (searchFor <= 0x7F && replaceWith <= 0x7F) {
        for (char& ch : m_string) {
            if (ch == static_cast<char8>(searchFor)) {
                ch = static_cast<char8>(replaceWith);
            }
        }
    } else {
        String searchForStr;
        String replaceWithStr;
        utf8::append(searchFor, std::back_inserter(searchForStr.m_string));
        utf8::append(replaceWith, std::back_inserter(replaceWithStr.m_string));
        return replace(searchForStr, replaceWithStr);
    }
}

void String::replace(const String& searchFor, const String& replaceWith) {
    std::size_t step = replaceWith.m_string.size();
    std::size_t len = searchFor.m_string.size();
    // Start the iterator at the beginning of the sequence
    size_t find_it_pos = 0;
    // Replace each occurrence of search
    while (true) {
        // Search the existence of the string searchFor in the range
        // [m_string.begin() + find_it_pos, m_string.end())
        auto find_it(std::search(m_string.begin() + find_it_pos, m_string.end(), searchFor.m_string.cbegin(),
                                 searchFor.m_string.cend()));
        // Check if we reach the end of the string
        if (find_it == m_string.end()) {
            return;
        }
        // Retrieve the current iterator position
        find_it_pos = find_it - m_string.begin();
        // Replace all the range between [find_it, find_it + len) with the
        // string in replaceWith
        m_string.replace(find_it, find_it + len, replaceWith.m_string);
        // Add an additional step to continue the search
        find_it_pos += step;
    }
}

String String::subString(std::size_t position, std::size_t length) const {
    // Iterate to the start codepoint
    auto start_it(m_string.begin());
    for (std::size_t i = 0; i < position; i++) {
        ENGINE_TRY {
            utf8::next(start_it, m_string.end());
        }
        ENGINE_CATCH(utf8::not_enough_room) {
            ENGINE_THROW(std::out_of_range("the specified position is out of the string range"));
        }
    }
    // Iterate to the end codepoint
    auto end_it(start_it);
    for (std::size_t i = 0; i < length; i++) {
        utf8::next(end_it, m_string.end());
        if (end_it == m_string.end()) {
            break;
        }
    }
    return String::FromUtf8(&(*start_it), &(*end_it));
}

const char8* String::getData() const {
    return m_string.data();
}

/////////////////////////// TODO

String::iterator String::begin() {
    return m_string.begin();
}

String::const_iterator String::begin() const {
    return m_string.begin();
}

String::iterator String::end() {
    return m_string.end();
}

String::const_iterator String::end() const {
    return m_string.end();
}

/////////////////////////// DOTO

bool operator==(const String& left, const String& right) {
    return left.m_string == right.m_string;
}

bool operator==(const String& left, const char8* right) {
    return left.m_string == right;
}

bool operator==(const char8* left, const String& right) {
    return left == right.m_string;
}

bool operator!=(const String& left, const String& right) {
    return !(left == right);
}

bool operator!=(const String& left, const char8* right) {
    return !(left == right);
}

bool operator!=(const char8* left, const String& right) {
    return !(left == right);
}

bool operator<(const String& left, const String& right) {
    return left.m_string < right.m_string;
}

bool operator<(const String& left, const char8* right) {
    return left.m_string < right;
}

bool operator<(const char8* left, const String& right) {
    return left < right.m_string;
}

bool operator>(const String& left, const String& right) {
    return right < left;
}

bool operator>(const String& left, const char8* right) {
    return right < left;
}

bool operator>(const char8* left, const String& right) {
    return right < left;
}

bool operator<=(const String& left, const String& right) {
    return !(right < left);
}

bool operator<=(const String& left, const char8* right) {
    return !(right < left);
}

bool operator<=(const char8* left, const String& right) {
    return !(right < left);
}

bool operator>=(const String& left, const String& right) {
    return !(left < right);
}

bool operator>=(const String& left, const char8* right) {
    return !(left < right);
}

bool operator>=(const char8* left, const String& right) {
    return !(left < right);
}

String operator+(const String& left, const String& right) {
    String string = left;
    return string += right;
}

String operator+(const String& left, const char8* right) {
    String string = left;
    return string += right;
}

String operator+(const char8* left, const String& right) {
    String string = left;
    return string += right;
}

String operator+(const String& left, char8 right) {
    String string = left;
    return string += right;
}

String operator+(char8 left, const String& right) {
    String string = left;
    return string += right;
}

std::ostream& operator<<(std::ostream& os, const String& str) {
    return os << str.toUtf8();
}

}  // namespace engine
