#pragma once

#include <mathfu/glsl_mappings.h>  // TMP

namespace engine {

namespace math {

template <typename T>
inline T Radians(T degrees) {
    static_assert(std::numeric_limits<T>::is_iec559, "'Radians' only accept floating-point input");
    return degrees * static_cast<T>(0.01745329251994329576923690768489);
}

template <typename T>
inline T Degrees(T radians) {
    static_assert(std::numeric_limits<T>::is_iec559, "'Degrees' only accept floating-point input");
    return radians * static_cast<T>(57.295779513082320876798154814105);
}

}  // namespace math

}  // namespace engine
