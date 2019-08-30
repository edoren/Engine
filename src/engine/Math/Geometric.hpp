#pragma once

#include "Vector2.hpp"
#include "Vector3.hpp"
#include "Vector4.hpp"

#include <cmath>

#include <type_traits>

namespace engine {

namespace math {

template <typename T>
inline Vector3<T> Cross(const Vector3<T>& v1, const Vector3<T>& v2) {
    static_assert(std::numeric_limits<T>::is_iec559,
                  "'Cross' only accept floating-point inputs");
    return Vector3<T>(v1.y * v2.z - v2.y * v1.z, v1.z * v2.x - v2.z * v1.x,
                      v1.x * v2.y - v2.x * v1.y);
}

template <typename T>
inline T Dot(const Vector2<T>& v1, const Vector2<T>& v2) {
    static_assert(std::numeric_limits<T>::is_iec559,
                  "'Dot' only accept floating-point inputs");
    return v1.x * v2.x + v1.y * v2.y;
}

template <typename T>
inline T Dot(const Vector3<T>& v1, const Vector3<T>& v2) {
    static_assert(std::numeric_limits<T>::is_iec559,
                  "'Dot' only accept floating-point inputs");
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

template <typename T>
inline T Dot(const Vector4<T>& v1, const Vector4<T>& v2) {
    static_assert(std::numeric_limits<T>::is_iec559,
                  "'Dot' only accept floating-point inputs");
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w;
}

template <typename T, template <typename> class VecType>
inline T LengthSquared(const VecType<T>& v) {
    static_assert(std::numeric_limits<T>::is_iec559,
                  "'LengthSquared' only accept floating-point inputs");
    return Dot(v, v);
}

template <typename T, template <typename> class VecType>
inline T Length(const VecType<T>& v) {
    static_assert(std::numeric_limits<T>::is_iec559,
                  "'Length' only accept floating-point inputs");
    return static_cast<T>(sqrt(LengthSquared(v)));
}

template <typename T, template <typename> class VecType>
inline VecType<T> Normalize(const VecType<T>& v) {
    static_assert(std::numeric_limits<T>::is_iec559,
                  "'Normalize' only accept floating-point inputs");
    return v * (static_cast<T>(1) / Length(v));
}

}  // namespace math

}  // namespace engine
