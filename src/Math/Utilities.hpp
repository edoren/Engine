#pragma once

namespace engine {

namespace math {

template <typename T>
constexpr const T& Clamp(const T& v, const T& lo, const T& hi) {
    return (v < lo) ? lo : (v > hi) ? hi : v;
}

}  // namespace math

}  // namespace engine
