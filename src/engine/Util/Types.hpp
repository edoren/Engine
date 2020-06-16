#pragma once

#include <cstdint>
#include <cwchar>

namespace engine {

// Integer types
using uint8 = uint8_t;
using uint16 = uint16_t;
using uint32 = uint32_t;
using uint64 = uint64_t;
using int8 = int8_t;
using int16 = int16_t;
using int32 = int32_t;
using int64 = int64_t;
using uintptr = uintptr_t;

// Byte type
using byte = uint8;

// Char types for UTF-8, UTF-16 and UTF-32 respectively
using char8 = char;
using char16 = char16_t;
using char32 = char32_t;

// This type represents a wide char, its size and encoding
// varies depending on the platform
using wchar = wchar_t;

}  // namespace engine
