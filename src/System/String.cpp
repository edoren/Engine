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

#include <cstring>

#include <utf8.h>

namespace engine {

const std::size_t String::InvalidPos = std::string::npos;

String::String() {}

String::String(char asciiChar) {
    string_ += asciiChar;
}

String::String(char16_t utf16Char) {
    char16_t* iterator = &utf16Char;
    utf8::utf16to8(iterator, iterator + 1, std::back_inserter(string_));
}

String::String(char32_t utf32Char) {
    char32_t* iterator = &utf32Char;
    utf8::utf32to8(iterator, iterator + 1, std::back_inserter(string_));
}

String::String(const char* utf8String) {
    if (utf8String) {
        std::size_t length = strlen(utf8String);
        if (length > 0 && utf8::is_valid(utf8String, utf8String + length)) {
            string_.assign(utf8String);
        };
    }
}

String::String(const std::string& utf8String) {
    if (utf8String.size() > 0 &&
        utf8::is_valid(utf8String.cbegin(), utf8String.cend())) {
        string_.assign(utf8String);
    };
}

String::String(const std::u16string& utf16String) {
    utf8::utf16to8(utf16String.cbegin(), utf16String.cend(),
                   std::back_inserter(string_));
}

String::String(const std::u32string& utf32String) {
    utf8::utf32to8(utf32String.cbegin(), utf32String.cend(),
                   std::back_inserter(string_));
}

String::String(const String& other) : string_(other.string_) {}

String::String(String&& other) : string_(std::move(other.string_)) {}

String::operator std::string() const {
    return ToUtf8();
}

String::operator std::u16string() const {
    return ToUtf16();
}

String::operator std::u32string() const {
    return ToUtf32();
}

const std::string& String::ToUtf8() const {
    return string_;
}

std::u16string String::ToUtf16() const {
    std::u16string output;
    utf8::utf8to16(string_.cbegin(), string_.cend(),
                   std::back_inserter(output));
    return output;
}

std::u32string String::ToUtf32() const {
    std::u32string output;
    utf8::utf8to32(string_.cbegin(), string_.cend(),
                   std::back_inserter(output));
    return output;
}

String& String::operator=(const String& right) {
    string_ = right.string_;
    return *this;
}

String& String::operator=(String&& right) {
    string_ = std::move(right.string_);
    return *this;
}

String& String::operator+=(const String& right) {
    string_ += right.string_;
    return *this;
}

char String::operator[](std::size_t index) const {
    return string_[index];
}

char& String::operator[](std::size_t index) {
    return string_[index];
}

void String::Clear() {
    string_.clear();
}

std::size_t String::GetSize() const {
    return utf8::distance(string_.begin(), string_.end());
}

bool String::IsEmpty() const {
    return string_.empty();
}

void String::Erase(std::size_t position, std::size_t count) {
    std::string::iterator start_it(string_.begin());
    for (std::size_t i = 0; i < position; ++i) {
        try {
            utf8::next(start_it, string_.end());
        } catch (utf8::not_enough_room) {
            throw std::out_of_range(
                "the specified position is out of the string range");
        }
    }
    std::string::iterator end_it(start_it);
    for (std::size_t i = 0; i < count; ++i) {
        utf8::next(end_it, string_.end());
        if (end_it == string_.end()) break;
    }
    string_.erase(start_it, end_it);
}

void String::Insert(std::size_t position, const String& str) {
    std::string::iterator start_it(string_.begin());
    for (std::size_t i = 0; i < position; ++i) {
        try {
            utf8::next(start_it, string_.end());
        } catch (utf8::not_enough_room) {
            throw std::out_of_range(
                "the specified position is out of the string range");
        }
    }
    string_.insert(start_it, str.string_.cbegin(), str.string_.cend());
}
// TODO
std::size_t String::Find(const String& str, std::size_t start) const {
    return string_.find(str.string_, start);
}
// TODO
void String::Replace(std::size_t position, std::size_t length,
                     const String& replaceWith) {
    string_.replace(position, length, replaceWith.string_);
}
// TODO
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
    std::string::const_iterator start_it(string_.begin());
    for (std::size_t i = 0; i < position; ++i) {
        try {
            utf8::next(start_it, string_.end());
        } catch (utf8::not_enough_room) {
            throw std::out_of_range(
                "the specified position is out of the string range");
        }
    }
    std::string::const_iterator end_it(start_it);
    for (std::size_t i = 0; i < length; ++i) {
        utf8::next(end_it, string_.end());
        if (end_it == string_.end()) break;
    }
    return String::FromUtf8(start_it, end_it);
}

const std::string::value_type* String::GetData() const {
    return string_.data();
}

/////////////////////////// TODO

String::iterator String::Begin() {
    return string_.begin();
}

String::const_iterator String::Begin() const {
    return string_.begin();
}

String::iterator String::End() {
    return string_.end();
}

String::const_iterator String::End() const {
    return string_.end();
}

/////////////////////////// DOTO

bool operator==(const String& left, const String& right) {
    return left.string_ == right.string_;
}

bool operator!=(const String& left, const String& right) {
    return !(left == right);
}

bool operator<(const String& left, const String& right) {
    return left.string_ < right.string_;
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
