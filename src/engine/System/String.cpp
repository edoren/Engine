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

#include <Util/UTF.hpp>

#include <algorithm>

namespace engine {

const std::size_t String::sInvalidPos = std::basic_string<char8>::npos;

String::String() = default;

String::String(char8 asciiChar) {
    m_string += asciiChar;
}

String::String(char16 utf16Char) {
    char16* ptr = &utf16Char;
    utf::UtfToUtf<16, 8>(ptr, ptr + 1, &m_string);
}

String::String(char32 utf32Char) {
    char32* ptr = &utf32Char;
    utf::UtfToUtf<32, 8>(ptr, ptr + 1, &m_string);
}

String::String(const char8* utf8String) {
    if (utf8String && utf8String[0] != 0) {
        std::size_t length = std::strlen(utf8String);
        if (length > 0) {
            if (utf::IsValidUTF<8>(utf8String, utf8String + length)) {
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
        utf::UtfToUtf<16, 8>(utf16String, utf16String + length, &m_string);
    }
}

String::String(const char32* utf32String) {
    if (utf32String && utf32String[0] != 0) {
        // Find the lenght
        const char32* it = utf32String;
        while (*(++it) != 0) {
        }
        std::size_t length = it - utf32String;
        utf::UtfToUtf<32, 8>(utf32String, utf32String + length, &m_string);
    }
}

String::String(const wchar* wideString) {
    if (wideString && wideString[0] != 0) {
        std::size_t length = std::wcslen(wideString);
#if PLATFORM_IS(PLATFORM_WINDOWS)
        utf::UtfToUtf<16, 8>(wideString, wideString + length, &m_string);
#else
        utf::UtfToUtf<32, 8>(wideString, wideString + length, &m_string);
#endif
    }
}

String::String(const std::basic_string<char8>& utf8String) {
    if (!utf8String.empty()) {
        if (utf::IsValidUTF<8>(utf8String.cbegin(), utf8String.cend())) {
            m_string.assign(utf8String);
        } else {
            ENGINE_THROW(std::runtime_error("invalid utf8 convertion."));
        }
    };
}

String::String(std::basic_string<char8>&& utf8String) {
    if (!utf8String.empty()) {
        if (utf::IsValidUTF<8>(utf8String.cbegin(), utf8String.cend())) {
            m_string = std::move(utf8String);
        } else {
            ENGINE_THROW(std::runtime_error("invalid utf8 convertion."));
        }
    };
}

String::String(const std::basic_string<char16>& utf16String) {
    utf::UtfToUtf<16, 8>(utf16String.cbegin(), utf16String.cend(), &m_string);
}

String::String(const std::basic_string<char32>& utf32String) {
    utf::UtfToUtf<32, 8>(utf32String.cbegin(), utf32String.cend(), &m_string);
}

String::String(const std::basic_string<wchar>& wideString) {
#if PLATFORM_IS(PLATFORM_WINDOWS)
    utf::UtfToUtf<16, 8>(wideString.cbegin(), wideString.cend(), &m_string);
#else
    utf::UtfToUtf<32, 8>(wideString.cbegin(), wideString.cend(), &m_string);
#endif
}

String::String(const String& other) = default;

String::String(String&& other) noexcept : m_string(std::move(other.m_string)) {}

String::~String() = default;

String String::FromUtf8(const char8* begin, const char8* end) {
    String string;
    if (utf::IsValidUTF<8>(begin, end)) {
        string.m_string.assign(begin, end);
    } else {
        ENGINE_THROW(std::runtime_error("invalid utf8 convertion."));
    }
    return string;
}

String String::FromUtf8(iterator begin, iterator end) {
    return FromUtf8(begin.getPtr(), end.getPtr());
}

String String::FromUtf16(const char16* begin, const char16* end) {
    String string;
    utf::UtfToUtf<16, 8>(begin, end, &string.m_string);
    return string;
}

String String::FromUtf32(const char32* begin, const char32* end) {
    String string;
    utf::UtfToUtf<32, 8>(begin, end, &string.m_string);
    return string;
}

String String::FromWide(const wchar* begin, const wchar* end) {
#if PLATFORM_IS(PLATFORM_WINDOWS)
    return FromUtf16(reinterpret_cast<const char16*>(begin), reinterpret_cast<const char16*>(end));
#else
    return FromUtf32(reinterpret_cast<const char32*>(begin), reinterpret_cast<const char32*>(end));
#endif
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
    utf::UtfToUtf<8, 16>(m_string.cbegin(), m_string.cend(), &output);
    return output;
}

std::basic_string<char32> String::toUtf32() const {
    std::basic_string<char32> output;
    utf::UtfToUtf<8, 32>(m_string.cbegin(), m_string.cend(), &output);
    return output;
}

std::basic_string<wchar> String::toWide() const {
    std::basic_string<wchar> output;
#if PLATFORM_IS(PLATFORM_WINDOWS)
    utf::UtfToUtf<8, 16>(m_string.cbegin(), m_string.cend(), &output);
#else
    utf::UtfToUtf<8, 32>(m_string.cbegin(), m_string.cend(), &output);
#endif
    return output;
}

String& String::operator=(const String& right) = default;

String& String::operator=(const char8* right) {
    m_string = right;
    return *this;
}

String& String::operator=(String&& right) noexcept {
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

String& String::operator+=(const utf::CodeUnit<8>& right) {
    // TODO: Missing test
    m_string.reserve(m_string.size() + right.getSize());
    for (auto data : right) {
        m_string.push_back(static_cast<char8>(data));
    }
    return *this;
}

utf::CodeUnit<8> String::operator[](std::size_t index) const {
    // TODO: throw error
    return (begin() + index)->getUnit();
}

void String::clear() {
    m_string.clear();
}

std::size_t String::getSize() const {
    return utf::GetSizeUTF<8>(m_string.begin(), m_string.end());
}

bool String::isEmpty() const {
    return m_string.empty();
}

void String::erase(std::size_t position, std::size_t count) {
    // Iterate to the start codepoint
    auto startIt(m_string.begin());
    for (std::size_t i = 0; i < position; i++) {
        auto nextIt = utf::NextUTF<8>(startIt, m_string.end());
        if (nextIt == m_string.end()) {
            ENGINE_THROW(std::out_of_range("the specified position is out of the string range"));
        }
        startIt = nextIt;
    }
    // Iterate to the end codepoint
    auto endIt(startIt);
    for (std::size_t i = 0; i < count; i++) {
        endIt = utf::NextUTF<8>(endIt, m_string.end());
        if (endIt == m_string.end()) {
            break;
        }
    }
    m_string.erase(startIt, endIt);
}

void String::insert(std::size_t position, const String& str) {
    // Iterate to the start codepoint
    auto startIt(m_string.begin());
    for (std::size_t i = 0; i < position; i++) {
        auto nextIt = utf::NextUTF<8>(startIt, m_string.end());
        if (nextIt == m_string.end()) {
            ENGINE_THROW(std::out_of_range("the specified position is out of the string range"));
        }
        startIt = nextIt;
    }
    m_string.insert(startIt, str.m_string.cbegin(), str.m_string.cend());
}

std::size_t String::find(const String& str, std::size_t start) const {
    // Iterate to the start codepoint
    auto startIt(m_string.cbegin());
    for (std::size_t i = 0; i < start; i++) {
        startIt = utf::NextUTF<8>(startIt, m_string.cend());
        if (startIt == m_string.cend()) {
            return sInvalidPos;
        }
    }
    // Find the string
    auto findIt(std::search(startIt, m_string.cend(), str.m_string.cbegin(), str.m_string.cend()));
    return (findIt == m_string.cend()) ? sInvalidPos : utf::GetSizeUTF<8>(m_string.cbegin(), findIt);
}

std::size_t String::findFirstOf(const String& str, std::size_t pos) const {
    size_t strSize = getSize();

    if (pos >= strSize) {
        return sInvalidPos;
    }

    // Iterate to the start codepoint
    auto startIt(m_string.cbegin());
    for (std::size_t i = 0; i < pos; i++) {
        startIt = utf::NextUTF<8>(startIt, m_string.cend());
        if (startIt == m_string.cend()) {
            return sInvalidPos;
        }
    }

    // Find one of the UTF-8 codepoints
    auto endIt(startIt);
    while (true) {
        if (startIt == m_string.cend()) {
            return sInvalidPos;
        }
        endIt = utf::NextUTF<8>(endIt, m_string.cend());
        auto findIt(std::search(str.m_string.cbegin(), str.m_string.cend(), startIt, endIt));
        if (findIt != str.m_string.cend()) {
            return utf::GetSizeUTF<8>(m_string.cbegin(), startIt);
        }
        startIt = endIt;
    }
}

std::size_t String::findLastOf(const String& str, std::size_t pos) const {
    // Iterate to the start codepoint
    auto startIt(m_string.cbegin());
    if (pos == sInvalidPos) {
        startIt = m_string.cend();
    } else {
        for (std::size_t i = 0; i < pos + 1; i++) {
            startIt = utf::NextUTF<8>(startIt, m_string.cend());
            if (startIt == m_string.cend()) {
                break;
            }
        }
    }

    // Find one of the UTF-8 codepoints
    auto endIt(startIt);
    while (true) {
        if (startIt == m_string.cbegin()) {
            return sInvalidPos;
        }
        endIt = utf::PriorUTF<8>(endIt, m_string.cbegin());
        auto findIt(std::search(str.m_string.cbegin(), str.m_string.cend(), endIt, startIt));
        if (findIt != str.m_string.cend()) {
            return utf::GetSizeUTF<8>(m_string.cbegin(), endIt);
        }
        startIt = endIt;
    }

    return sInvalidPos;
}

void String::replace(std::size_t position, std::size_t length, const String& replaceWith) {
    // Iterate to the start codepoint
    auto startIt(m_string.begin());
    for (std::size_t i = 0; i < position; i++) {
        auto nextIt = utf::NextUTF<8>(startIt, m_string.end());
        if (nextIt == m_string.end()) {
            ENGINE_THROW(std::out_of_range("the specified position is out of the string range"));
        }
        startIt = nextIt;
    }
    // Iterate to the end codepoint
    auto endIt(startIt);
    for (std::size_t i = 0; i < length; i++) {
        endIt = utf::NextUTF<8>(endIt, m_string.end());
        if (endIt == m_string.end()) {
            break;
        }
    }
    m_string.replace(startIt, endIt, replaceWith.m_string);
}

void String::replace(uint32 searchFor, uint32 replaceWith) {
    if (searchFor == replaceWith) {
        return;
    }
    if (searchFor <= 0x7F && replaceWith <= 0x7F) {
        for (auto& ch : m_string) {
            if (ch == static_cast<char8>(searchFor)) {
                ch = static_cast<char8>(replaceWith);
            }
        }
    } else {
        String searchForStr;
        String replaceWithStr;
        utf::AppendToUtf<8>(searchFor, &searchForStr.m_string);
        utf::AppendToUtf<8>(replaceWith, &replaceWithStr.m_string);
        replace(searchForStr, replaceWithStr);
    }
}

void String::replace(const String& searchFor, const String& replaceWith) {
    std::size_t step = replaceWith.m_string.size();
    std::size_t len = searchFor.m_string.size();
    // Start the iterator at the beginning of the sequence
    size_t findItPos = 0;
    // Replace each occurrence of search
    while (true) {
        // Search the existence of the string searchFor in the range
        // [m_string.begin() + find_it_pos, m_string.end())
        auto findIt(std::search(m_string.begin() + findItPos, m_string.end(), searchFor.m_string.cbegin(),
                                searchFor.m_string.cend()));
        // Check if we reach the end of the string
        if (findIt == m_string.end()) {
            return;
        }
        // Retrieve the current iterator position
        findItPos = findIt - m_string.begin();
        // Replace all the range between [find_it, find_it + len) with the
        // string in replaceWith
        m_string.replace(findIt, findIt + len, replaceWith.m_string);
        // Add an additional step to continue the search
        findItPos += step;
    }
}

String String::subString(std::size_t position, std::size_t length) const {
    // Iterate to the start codepoint
    auto startIt(m_string.begin());
    for (std::size_t i = 0; i < position; i++) {
        auto nextIt = utf::NextUTF<8>(startIt, m_string.end());
        if (nextIt == m_string.end()) {
            ENGINE_THROW(std::out_of_range("the specified position is out of the string range"));
        }
        startIt = nextIt;
    }
    // Iterate to the end codepoint
    auto endIt(startIt);
    for (std::size_t i = 0; i < length; i++) {
        endIt = utf::NextUTF<8>(endIt, m_string.end());
        if (endIt == m_string.end()) {
            break;
        }
    }
    return String::FromUtf8(&(*startIt), &(*endIt));
}

const char8* String::getData() const {
    return m_string.data();
}

String::iterator String::begin() {
    return String::iterator(std::make_pair(m_string.data(), m_string.data() + m_string.size()), m_string.data());
}

String::const_iterator String::begin() const {
    return String::const_iterator(std::make_pair(m_string.data(), m_string.data() + m_string.size()), m_string.data());
}

String::iterator String::end() {
    return String::iterator(std::make_pair(m_string.data(), m_string.data() + m_string.size()),
                            m_string.data() + m_string.size());
}

String::const_iterator String::end() const {
    return String::const_iterator(std::make_pair(m_string.data(), m_string.data() + m_string.size()),
                                  m_string.data() + m_string.size());
}

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
