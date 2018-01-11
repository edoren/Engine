#pragma once

#include <mathfu/glsl_mappings.h>  // TMP

#include <Math/Matrix3x3.hpp>
#include <Math/Matrix4x4.hpp>
#include <Math/Vector3.hpp>
#include <Math/Vector4.hpp>

namespace engine {

namespace math {

template <typename T>
inline Matrix4x4<T> Perspective(T fovy, T aspect, T zNear, T zFar) {
    assert(std::abs(aspect - std::numeric_limits<T>::epsilon()) >
           static_cast<T>(0));

    T one = static_cast<T>(1);
    T two = static_cast<T>(2);

    T const tan_half_fovy = static_cast<T>(tan(fovy / two));

    return Matrix4x4<T>(
        one / (aspect * tan_half_fovy), 0.f, 0.f, 0.f,
        0.f, one / (tan_half_fovy), 0.f, 0.f,
        0.f, 0.f, -(zFar + zNear) / (zFar - zNear), -one,
        0.f, 0.f, -(two * zFar * zNear) / (zFar - zNear), 0.f
    );
}

template <typename T>
inline Matrix4x4<T> Ortho(T left, T right, T bottom, T top, T zNear, T zFar) {
    T two = static_cast<T>(2);
    Matrix4x4<T> result(
        two / (right - left), 0.f, 0.f, 0.f, 0.f, two / (top - bottom), 0.f, 0.f,
        0.f, 0.f, -two / (zFar - zNear), 0.f,
        -(right + left) / (right - left), -(top + bottom) / (top - bottom), -(zFar + zNear) / (zFar - zNear)
    );
    return result;
}

template <typename T>
inline Matrix4x4<T> LookAt(const Vector3<T>& position,
                           const Vector3<T>& target,
                           const Vector3<T>& up) {
    const Vector3<T> f(Normalize(position - target));
    const Vector3<T> s(Normalize(Cross(up, f)));
    const Vector3<T> u(Cross(f, s));

    return Matrix4x4<T>(
        s.x, u.x, f.x, 0.f,
        s.y, u.y, f.y, 0.f,
        s.z, u.z, f.z, 0.f,
        -Dot(s, position), -Dot(u, position), -Dot(f, position), 1.f
    );
}

template <typename T>
inline Matrix4x4<T> Rotate(T angle, const Vector3<T>& v) {
    static_assert(std::numeric_limits<T>::is_iec559, "'Rotate' only accept floating-point inputs");

    T c = static_cast<T>(cos(angle));
    T s = static_cast<T>(sin(angle));

    Vector3<T> axis(Normalize(v));
    Vector3<T> temp(axis * (T(1) - c));

    Matrix4x4<T> result(
        c + axis.x * temp.x, axis.y * temp.x + axis.z * s, axis.z * temp.x - axis.y * s, 0.f,
        axis.x * temp.y - axis.z * s, c + axis.y * temp.y, axis.z * temp.y + axis.x * s, 0.f,
        axis.x * temp.z + axis.y * s, axis.y * temp.z - axis.x * s, c + axis.z * temp.z, 0.f,
        0.f, 0.f, 0.f, 1.f
    );

    return result;
}

template <typename T>
inline Matrix4x4<T> Scale(T x, T y, T z) {
    return Matrix4x4<T>(
        x, 0.f, 0.f, 0.f,
        0.f, y, 0.f, 0.f,
        0.f, 0.f, z, 0.f,
        0.f, 0.f, 0.f, 1.f
    );
}

template <typename T>
inline Matrix4x4<T> Scale(const Vector3<T>& v) {
    return Scale(v.x, v.y, v.z);
}

template <typename T>
inline Matrix4x4<T> Translate(T x, T y, T z) {
    return Matrix4x4<T>(
        1.f, 0.f, 0.f, 0.f,
        0.f, 1.f, 0.f, 0.f,
        0.f, 0.f, 1.f, 0.f,
        x, y, z, 1.f
    );
}

template <typename T>
inline Matrix4x4<T> Translate(const Vector3<T>& v) {
    return Translate(v.x, v.y, v.z);
}

}  // namespace math

}  // namespace engine