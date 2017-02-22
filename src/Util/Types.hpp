#pragma once

#include <stdint.h>
#include <wchar.h>

namespace engine {

// Integer types
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

// Byte type
typedef uint8 byte;

// Char types for UTF-8, UTF-16 and UTF-32 respectively
typedef char char8;
typedef char16_t char16;
typedef char32_t char32;

// This type represents a wide char, its size and encoding
// varies depending on the platform
typedef wchar_t wchar;

}  // namespace engine
