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

const String::size_type String::sInvalidPos = std::basic_string<char>::npos;

String::String() = default;

String::String(char asciiChar) {
    m_string += asciiChar;
}

String::String(char16 utf16Char) {
    char16* ptr = &utf16Char;
    utf::UtfToUtf<utf::UTF_16, utf::UTF_8>(ptr, ptr + 1, &m_string);
}

String::String(char32 utf32Char) {
    char32* ptr = &utf32Char;
    utf::UtfToUtf<utf::UTF_32, utf::UTF_8>(ptr, ptr + 1, &m_string);
}

String::String(const char* utf8String) {
    if (utf8String && utf8String[0] != 0) {
        size_type length = std::strlen(utf8String);
        if (length > 0) {
            if (utf::IsValid<utf::UTF_8>(utf8String, utf8String + length)) {
                m_string.assign(utf8String);
            } else {
                ENGINE_THROW(std::runtime_error("invalid utf8 convertion."));
            }
        };
    }
}

String::String(const char8* utf8String) : String(reinterpret_cast<const char*>(utf8String)) {}

String::String(const char16* utf16String) {
    if (utf16String && utf16String[0] != 0) {
        // Find the lenght
        const char16* it = utf16String;
        while (*(++it) != 0) {
        }
        size_type length = it - utf16String;
        utf::UtfToUtf<utf::UTF_16, utf::UTF_8>(utf16String, utf16String + length, &m_string);
    }
}

String::String(const char32* utf32String) {
    if (utf32String && utf32String[0] != 0) {
        // Find the lenght
        const char32* it = utf32String;
        while (*(++it) != 0) {
        }
        size_type length = it - utf32String;
        utf::UtfToUtf<utf::UTF_32, utf::UTF_8>(utf32String, utf32String + length, &m_string);
    }
}

String::String(const wchar* wideString) {
    if (wideString && wideString[0] != 0) {
        size_type length = std::wcslen(wideString);
#if PLATFORM_IS(PLATFORM_WINDOWS)
        utf::UtfToUtf<utf::UTF_16, utf::UTF_8>(wideString, wideString + length, &m_string);
#else
        utf::UtfToUtf<utf::UTF_32, utf::UTF_8>(wideString, wideString + length, &m_string);
#endif
    }
}

String::String(const std::basic_string<char>& utf8String) {
    if (!utf8String.empty()) {
        if (utf::IsValid<utf::UTF_8>(utf8String.cbegin(), utf8String.cend())) {
            m_string.assign(utf8String);
        } else {
            ENGINE_THROW(std::runtime_error("invalid utf8 convertion."));
        }
    };
}

String::String(std::basic_string<char>&& utf8String) {
    if (!utf8String.empty()) {
        if (utf::IsValid<utf::UTF_8>(utf8String.cbegin(), utf8String.cend())) {
            m_string = std::move(utf8String);
        } else {
            ENGINE_THROW(std::runtime_error("invalid utf8 convertion."));
        }
    };
}

String::String(const std::basic_string<char8>& utf8String) {
    if (!utf8String.empty()) {
        if (utf::IsValid<utf::UTF_8>(utf8String.cbegin(), utf8String.cend())) {
            m_string.assign(reinterpret_cast<const char*>(utf8String.data()), utf8String.size());
        } else {
            ENGINE_THROW(std::runtime_error("invalid utf8 convertion."));
        }
    };
}

String::String(const std::basic_string<char16>& utf16String) {
    utf::UtfToUtf<utf::UTF_16, utf::UTF_8>(utf16String.cbegin(), utf16String.cend(), &m_string);
}

String::String(const std::basic_string<char32>& utf32String) {
    utf::UtfToUtf<utf::UTF_32, utf::UTF_8>(utf32String.cbegin(), utf32String.cend(), &m_string);
}

String::String(const std::basic_string<wchar>& wideString) {
#if PLATFORM_IS(PLATFORM_WINDOWS)
    utf::UtfToUtf<utf::UTF_16, utf::UTF_8>(wideString.cbegin(), wideString.cend(), &m_string);
#else
    utf::UtfToUtf<utf::UTF_32, utf::UTF_8>(wideString.cbegin(), wideString.cend(), &m_string);
#endif
}

String::String(const String& other) = default;

String::String(String&& other) noexcept : m_string(std::move(other.m_string)) {}

String::~String() = default;

String String::FromUtf8(const char* begin, const char* end) {
    String string;
    if (utf::IsValid<utf::UTF_8>(begin, end)) {
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
    utf::UtfToUtf<utf::UTF_16, utf::UTF_8>(begin, end, &string.m_string);
    return string;
}

String String::FromUtf32(const char32* begin, const char32* end) {
    String string;
    utf::UtfToUtf<utf::UTF_32, utf::UTF_8>(begin, end, &string.m_string);
    return string;
}

String String::FromWide(const wchar* begin, const wchar* end) {
#if PLATFORM_IS(PLATFORM_WINDOWS)
    return FromUtf16(reinterpret_cast<const char16*>(begin), reinterpret_cast<const char16*>(end));
#else
    return FromUtf32(reinterpret_cast<const char32*>(begin), reinterpret_cast<const char32*>(end));
#endif
}

String::operator std::basic_string<char>() const {
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

const std::basic_string<char>& String::toUtf8() const {
    return m_string;
}

std::basic_string<char16> String::toUtf16() const {
    std::basic_string<char16> output;
    utf::UtfToUtf<utf::UTF_8, utf::UTF_16>(m_string.cbegin(), m_string.cend(), &output);
    return output;
}

std::basic_string<char32> String::toUtf32() const {
    std::basic_string<char32> output;
    utf::UtfToUtf<utf::UTF_8, utf::UTF_32>(m_string.cbegin(), m_string.cend(), &output);
    return output;
}

std::basic_string<wchar> String::toWide() const {
    std::basic_string<wchar> output;
#if PLATFORM_IS(PLATFORM_WINDOWS)
    utf::UtfToUtf<utf::UTF_8, utf::UTF_16>(m_string.cbegin(), m_string.cend(), &output);
#else
    utf::UtfToUtf<utf::UTF_8, utf::UTF_32>(m_string.cbegin(), m_string.cend(), &output);
#endif
    return output;
}

String& String::operator=(const String& right) = default;

String& String::operator=(const char* right) {
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

String& String::operator+=(const char* right) {
    m_string += right;
    return *this;
}

String& String::operator+=(const char8* right) {
    m_string += reinterpret_cast<const char*>(right);
    return *this;
}

String& String::operator+=(char right) {
    if (right >= 0) {
        m_string += right;
    }
    return *this;
}

String& String::operator+=(char32 right) {
    utf::UtfToUtf<utf::UTF_32, utf::UTF_8>(&right, &right + 1, &m_string);
    return *this;
}

String& String::operator+=(const utf::CodeUnit<utf::UTF_8>& right) {
    // TODO: Missing test
    m_string.reserve(m_string.size() + right.getSize());
    for (auto data : right) {
        m_string.push_back(static_cast<char>(data));
    }
    return *this;
}

utf::CodeUnit<utf::UTF_8> String::operator[](size_type index) const {
    // TODO: throw error
    return (cbegin() + index)->get();
}

void String::clear() {
    m_string.clear();
}

String::size_type String::getSize() const {
    return utf::GetSize<utf::UTF_8>(m_string.begin(), m_string.end());
}

bool String::isEmpty() const {
    return m_string.empty();
}

void String::erase(size_type position, size_type count) {
    // Iterate to the start codepoint
    auto startIt(m_string.begin());
    for (size_type i = 0; i < position; i++) {
        auto nextIt = utf::Next<utf::UTF_8>(startIt, m_string.end());
        if (nextIt == m_string.end()) {
            ENGINE_THROW(std::out_of_range("the specified position is out of the string range"));
        }
        startIt = nextIt;
    }
    // Iterate to the end codepoint
    auto endIt(startIt);
    for (size_type i = 0; i < count; i++) {
        endIt = utf::Next<utf::UTF_8>(endIt, m_string.end());
        if (endIt == m_string.end()) {
            break;
        }
    }
    m_string.erase(startIt, endIt);
}

void String::insert(size_type position, const String& str) {
    // Iterate to the start codepoint
    auto startIt(m_string.begin());
    for (size_type i = 0; i < position; i++) {
        auto nextIt = utf::Next<utf::UTF_8>(startIt, m_string.end());
        if (nextIt == m_string.end()) {
            ENGINE_THROW(std::out_of_range("the specified position is out of the string range"));
        }
        startIt = nextIt;
    }
    m_string.insert(startIt, str.m_string.cbegin(), str.m_string.cend());
}

String::size_type String::find(const String& str, size_type start) const {
    // Iterate to the start codepoint
    auto startIt(m_string.cbegin());
    for (size_type i = 0; i < start; i++) {
        startIt = utf::Next<utf::UTF_8>(startIt, m_string.cend());
        if (startIt == m_string.cend()) {
            return sInvalidPos;
        }
    }
    // Find the string
    auto findIt(std::search(startIt, m_string.cend(), str.m_string.cbegin(), str.m_string.cend()));
    return (findIt == m_string.cend()) ? sInvalidPos : utf::GetSize<utf::UTF_8>(m_string.cbegin(), findIt);
}

String::size_type String::findFirstOf(const String& str, size_type pos) const {
    size_t strSize = getSize();

    if (pos >= strSize) {
        return sInvalidPos;
    }

    // Iterate to the start codepoint
    auto startIt(m_string.cbegin());
    for (size_type i = 0; i < pos; i++) {
        startIt = utf::Next<utf::UTF_8>(startIt, m_string.cend());
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
        endIt = utf::Next<utf::UTF_8>(endIt, m_string.cend());
        auto findIt(std::search(str.m_string.cbegin(), str.m_string.cend(), startIt, endIt));
        if (findIt != str.m_string.cend()) {
            return utf::GetSize<utf::UTF_8>(m_string.cbegin(), startIt);
        }
        startIt = endIt;
    }
}

String::size_type String::findLastOf(const String& str, size_type pos) const {
    // Iterate to the start codepoint
    auto startIt(m_string.cbegin());
    if (pos == sInvalidPos) {
        startIt = m_string.cend();
    } else {
        for (size_type i = 0; i < pos + 1; i++) {
            startIt = utf::Next<utf::UTF_8>(startIt, m_string.cend());
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
        endIt = utf::Prior<utf::UTF_8>(endIt, m_string.cbegin());
        auto findIt(std::search(str.m_string.cbegin(), str.m_string.cend(), endIt, startIt));
        if (findIt != str.m_string.cend()) {
            return utf::GetSize<utf::UTF_8>(m_string.cbegin(), endIt);
        }
        startIt = endIt;
    }

    return sInvalidPos;
}

void String::replace(size_type position, size_type length, const String& replaceWith) {
    // Iterate to the start codepoint
    auto startIt(m_string.begin());
    for (size_type i = 0; i < position; i++) {
        auto nextIt = utf::Next<utf::UTF_8>(startIt, m_string.end());
        if (nextIt == m_string.end()) {
            ENGINE_THROW(std::out_of_range("the specified position is out of the string range"));
        }
        startIt = nextIt;
    }
    // Iterate to the end codepoint
    auto endIt(startIt);
    for (size_type i = 0; i < length; i++) {
        endIt = utf::Next<utf::UTF_8>(endIt, m_string.end());
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
            if (ch == static_cast<char>(searchFor)) {
                ch = static_cast<char>(replaceWith);
            }
        }
    } else {
        String searchForStr(static_cast<char32>(searchFor));
        String replaceWithStr(static_cast<char32>(replaceWith));
        replace(searchForStr, replaceWithStr);
    }
}

void String::replace(const String& searchFor, const String& replaceWith) {
    size_type step = replaceWith.m_string.size();
    size_type len = searchFor.m_string.size();
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

String String::subString(size_type position, size_type length) const {
    // Iterate to the start codepoint
    auto startIt(m_string.begin());
    for (size_type i = 0; i < position; i++) {
        auto nextIt = utf::Next<utf::UTF_8>(startIt, m_string.end());
        if (nextIt == m_string.end()) {
            ENGINE_THROW(std::out_of_range("the specified position is out of the string range"));
        }
        startIt = nextIt;
    }
    // Iterate to the end codepoint
    auto endIt(startIt);
    for (size_type i = 0; i < length; i++) {
        endIt = utf::Next<utf::UTF_8>(endIt, m_string.end());
        if (endIt == m_string.end()) {
            break;
        }
    }
    return String::FromUtf8(&(*startIt), &(*endIt));
}

const char* String::getData() const {
    return m_string.data();
}

typename String::size_type String::getDataSize() const {
    return m_string.size();
}

String::iterator String::begin() {
    auto maxRange = std::make_pair(m_string.data(), m_string.data() + m_string.size());
    return String::iterator(maxRange, maxRange.first);
}

String::const_iterator String::cbegin() const {
    auto maxRange = std::make_pair(m_string.data(), m_string.data() + m_string.size());
    return String::const_iterator(maxRange, maxRange.first);
}

String::iterator String::end() {
    auto maxRange = std::make_pair(m_string.data(), m_string.data() + m_string.size());
    return String::iterator(maxRange, maxRange.second);
}

String::const_iterator String::cend() const {
    auto maxRange = std::make_pair(m_string.data(), m_string.data() + m_string.size());
    return String::const_iterator(maxRange, maxRange.second);
}

String::reverse_iterator String::rbegin() {
    return std::reverse_iterator(end());
}

String::const_reverse_iterator String::crbegin() const {
    return std::reverse_iterator(cend());
}

String::reverse_iterator String::rend() {
    return std::reverse_iterator(begin());
}

String::const_reverse_iterator String::crend() const {
    return std::reverse_iterator(cbegin());
}

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

bool operator==(const String& left, const char* right) {
    return left.m_string == right;
}

bool operator!=(const String& left, const char* right) {
    return !(left == right);
}

bool operator<(const String& left, const char* right) {
    return left.m_string < right;
}

bool operator>(const String& left, const char* right) {
    return right < left;
}

bool operator<=(const String& left, const char* right) {
    return !(right < left);
}

bool operator>=(const String& left, const char* right) {
    return !(left < right);
}

bool operator==(const char* left, const String& right) {
    return left == right.m_string;
}

bool operator!=(const char* left, const String& right) {
    return !(left == right);
}

bool operator<(const char* left, const String& right) {
    return left < right.m_string;
}

bool operator>(const char* left, const String& right) {
    return right < left;
}

bool operator<=(const char* left, const String& right) {
    return !(right < left);
}

bool operator>=(const char* left, const String& right) {
    return !(left < right);
}


bool operator==(const String& left, const char8* right) {
    return left.m_string == reinterpret_cast<const char*>(right);
}

bool operator!=(const String& left, const char8* right) {
    return !(left == reinterpret_cast<const char*>(right));
}

bool operator<(const String& left, const char8* right) {
    return left.m_string < reinterpret_cast<const char*>(right);
}

bool operator>(const String& left, const char8* right) {
    return reinterpret_cast<const char*>(right) < left;
}

bool operator<=(const String& left, const char8* right) {
    return !(reinterpret_cast<const char*>(right) < left);
}

bool operator>=(const String& left, const char8* right) {
    return !(left < reinterpret_cast<const char*>(right));
}


bool operator==(const char8* left, const String& right) {
    return reinterpret_cast<const char*>(left) == right.m_string;
}

bool operator!=(const char8* left, const String& right) {
    return !(reinterpret_cast<const char*>(left) == right);
}

bool operator<(const char8* left, const String& right) {
    return reinterpret_cast<const char*>(left) < right.m_string;
}

bool operator>(const char8* left, const String& right) {
    return right < reinterpret_cast<const char*>(left);
}

bool operator<=(const char8* left, const String& right) {
    return !(right < reinterpret_cast<const char*>(left));
}

bool operator>=(const char8* left, const String& right) {
    return !(reinterpret_cast<const char*>(left) < right);
}

String operator+(const String& left, const String& right) {
    String string = left;
    return string += right;
}

String operator+(const String& left, const char* right) {
    String string = left;
    return string += right;
}

String operator+(const char* left, const String& right) {
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

String operator+(const String& left, char right) {
    String string = left;
    return string += right;
}

String operator+(char left, const String& right) {
    String string = left;
    return string += right;
}

std::ostream& operator<<(std::ostream& os, const String& str) {
    return os << str.toUtf8();
}

}  // namespace engine
