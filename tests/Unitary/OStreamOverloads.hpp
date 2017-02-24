#pragma once

#include <ostream>

#include <System/String.hpp>

namespace engine {

inline std::ostream& operator<<(std::ostream& os, const String& str) {
    return os << str.ToUtf8();
}

}  // namespace engine
