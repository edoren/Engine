#pragma once

#include <Util/Prerequisites.hpp>

#include <mathfu/glsl_mappings.h>  // TMP

#include <Math/Geometric.hpp>
#include <Math/Matrix3x3.hpp>
#include <Math/Matrix4x4.hpp>
#include <Math/Vector3.hpp>
#include <Math/Vector4.hpp>

// clang-format off

#define MATH_DEPTH_ZERO_TO_ONE         0x00000001
#define MATH_DEPTH_NEGATIVE_ONE_TO_ONE 0x00000002

#ifdef MATH_FORCE_DEPTH_ZERO_TO_ONE
    #define MATH_DEPTH_CLIP_SPACE MATH_DEPTH_ZERO_TO_ONE
#else
    #define MATH_DEPTH_CLIP_SPACE MATH_DEPTH_NEGATIVE_ONE_TO_ONE
#endif

#define MATH_LEFT_HANDED  0x00000001    // For DirectX, Metal, Vulkan
#define MATH_RIGHT_HANDED 0x00000002    // For OpenGL, default in GLM

#ifdef MATH_FORCE_LEFT_HANDED
    #define MATH_COORDINATE_SYSTEM MATH_LEFT_HANDED
#else
    #define MATH_COORDINATE_SYSTEM MATH_RIGHT_HANDED
#endif

// clang-format on

namespace engine {

namespace math {

template <typename T>
inline Matrix4x4<T> PerspectiveLH(T fovy, T aspect, T zNear, T zFar) {
    assert(std::abs(aspect - std::numeric_limits<T>::epsilon()) > static_cast<T>(0));

    T one = static_cast<T>(1);
    T two = static_cast<T>(2);

    T const tan_half_fovy = static_cast<T>(std::tan(fovy / two));

    // clang-format off
    return Matrix4x4<T>(
        one / (aspect * tan_half_fovy), 0.F, 0.F, 0.F,
        0.F, one / (tan_half_fovy), 0.F, 0.F,
        0.F, 0.F, (zFar + zNear) / (zFar - zNear), one,
        0.F, 0.F, -(two * zFar * zNear) / (zFar - zNear), 0.F
    );

    // // DEPTH_ZERO_TO_ONE
    // return Matrix4x4<T>(
    //     one / (aspect * tan_half_fovy), 0.f, 0.f, 0.f,
    //     0.f, one / (tan_half_fovy), 0.f, 0.f,
    //     0.f, 0.f, zFar / (zFar - zNear), one,
    //     0.f, 0.f, -(zFar * zNear) / (zFar - zNear), 0.f
    // );
    // clang-format on
}

template <typename T>
inline Matrix4x4<T> PerspectiveRH(T fovy, T aspect, T zNear, T zFar) {
    assert(std::abs(aspect - std::numeric_limits<T>::epsilon()) > static_cast<T>(0));

    T one = static_cast<T>(1);
    T two = static_cast<T>(2);

    T const tan_half_fovy = static_cast<T>(std::tan(fovy / two));

    // clang-format off
    return Matrix4x4<T>(
        one / (aspect * tan_half_fovy), 0.F, 0.F, 0.F,
        0.F, one / (tan_half_fovy), 0.F, 0.F,
        0.F, 0.F, -(zFar + zNear) / (zFar - zNear), -one,
        0.F, 0.F, -(two * zFar * zNear) / (zFar - zNear), 0.F
    );

    // // DEPTH_ZERO_TO_ONE
    // return Matrix4x4<T>(
    //     one / (aspect * tan_half_fovy), 0.f, 0.f, 0.f,
    //     0.f, one / (tan_half_fovy), 0.f, 0.f,
    //     0.f, 0.f, zFar / (zNear - zFar), -one,
    //     0.f, 0.f, -(zFar * zNear) / (zFar - zNear), 0.f
    // );
    // clang-format on
}

template <typename T>
inline Matrix4x4<T> Perspective(T fovy, T aspect, T zNear, T zFar) {
#if MATH_COORDINATE_SYSTEM == MATH_LEFT_HANDED
    return PerspectiveLH(fovy, aspect, zNear, zFar);
#else
    return PerspectiveRH(fovy, aspect, zNear, zFar);
#endif
}

template <typename T>
inline Matrix4x4<T> Ortho(T left, T right, T bottom, T top, T zNear, T zFar) {
    T one = static_cast<T>(1);
    T two = static_cast<T>(2);
    // clang-format off
    return Matrix4x4<T>(
                    two / (right - left),                              0.F,                              0.F, 0.F,
                                     0.F,             two / (top - bottom),                              0.F, 0.F,
                                     0.F,                              0.F,            -two / (zFar - zNear), 0.F,
        -(right + left) / (right - left), -(top + bottom) / (top - bottom), -(zFar + zNear) / (zFar - zNear), one
    );
    // clang-format on
}

template <typename T>
inline Matrix4x4<T> LookAtLH(const Vector3<T>& position, const Vector3<T>& target, const Vector3<T>& up) {
    const Vector3<T> f(Normalize(target - position));
    const Vector3<T> s(Normalize(Cross(up, f)));
    const Vector3<T> u(Cross(f, s));

    // clang-format off
    return Matrix4x4<T>(
        s.x, u.x, f.x, 0.F,
        s.y, u.y, f.y, 0.F,
        s.z, u.z, f.z, 0.F,
        -Dot(s, position), -Dot(u, position), -Dot(f, position), 1.F
    );
    // clang-format on
}

template <typename T>
inline Matrix4x4<T> LookAtRH(const Vector3<T>& position, const Vector3<T>& target, const Vector3<T>& up) {
    const Vector3<T> f(Normalize(target - position));
    const Vector3<T> s(Normalize(Cross(f, up)));
    const Vector3<T> u(Cross(s, f));

    // clang-format off
    return Matrix4x4<T>(
                      s.x,               u.x,             -f.x, 0.F,
                      s.y,               u.y,             -f.y, 0.F,
                      s.z,               u.z,             -f.z, 0.F,
        -Dot(s, position), -Dot(u, position), Dot(f, position), 1.F
    );
    // clang-format on
}

template <typename T>
inline Matrix4x4<T> LookAt(const Vector3<T>& position, const Vector3<T>& target, const Vector3<T>& up) {
#if MATH_COORDINATE_SYSTEM == MATH_LEFT_HANDED
    return LookAtLH(position, target, up);
#else
    return LookAtRH(position, target, up);
#endif
}

template <typename T>
inline Matrix4x4<T> Rotate(T angle, const Vector3<T>& v) {
    static_assert(std::numeric_limits<T>::is_iec559, "'Rotate' only accept floating-point inputs");

    T c = static_cast<T>(cos(angle));
    T s = static_cast<T>(sin(angle));

    Vector3<T> axis(Normalize(v));
    Vector3<T> temp(axis * (T(1) - c));

    // clang-format off
    return Matrix4x4<T>(
                 c + axis.x * temp.x, axis.y * temp.x + axis.z * s, axis.z * temp.x - axis.y * s, 0.F,
        axis.x * temp.y - axis.z * s,          c + axis.y * temp.y, axis.z * temp.y + axis.x * s, 0.F,
        axis.x * temp.z + axis.y * s, axis.y * temp.z - axis.x * s,          c + axis.z * temp.z, 0.F,
                                 0.F,                          0.F,                          0.F, 1.F
    );
    // clang-format on
}

template <typename T>
inline Matrix4x4<T> RotateAxisX(const T eulerAngle) {
    static_assert(std::numeric_limits<T>::is_iec559, "'Rotate' only accept floating-point inputs");

    T c = static_cast<T>(cos(eulerAngle));
    T s = static_cast<T>(sin(eulerAngle));

    // clang-format off
    return Matrix4x4<T>(
        1,  0, 0, 0,
        0,  c, s, 0,
        0, -s, c, 0,
        0,  0, 0, 1
    );
    // clang-format on
}

template <typename T>
inline Matrix4x4<T> RotateAxisY(const T eulerAngle) {
    static_assert(std::numeric_limits<T>::is_iec559, "'Rotate' only accept floating-point inputs");

    T c = static_cast<T>(cos(eulerAngle));
    T s = static_cast<T>(sin(eulerAngle));

    // clang-format off
    return Matrix4x4<T>(
        c, 0, -s, 0,
        0, 1,  0, 0,
        s, 0,  c, 0,
        0, 0,  0, 1
    );
    // clang-format on
}

template <typename T>
inline Matrix4x4<T> RotateAxisZ(const T eulerAngle) {
    static_assert(std::numeric_limits<T>::is_iec559, "'Rotate' only accept floating-point inputs");

    T c = static_cast<T>(cos(eulerAngle));
    T s = static_cast<T>(sin(eulerAngle));

    // clang-format off
    return Matrix4x4<T>(
         c, s, 0, 0,
        -s, c, 0, 0,
         0, 0, 1, 0,
         0, 0, 0, 1
    );
    // clang-format on
}

template <typename T>
inline Matrix4x4<T> Rotate(const Vector3<T>& eulerAngles) {
    static_assert(std::numeric_limits<T>::is_iec559, "'Rotate' only accept floating-point inputs");
    Matrix4x4<T> Rx = RotateAxisX(eulerAngles.x);
    Matrix4x4<T> Ry = RotateAxisX(eulerAngles.y);
    Matrix4x4<T> Rz = RotateAxisX(eulerAngles.z);
    return Rz * Ry * Rx;
}

template <typename T>
inline Matrix4x4<T> Scale(T x, T y, T z) {
    // clang-format off
    return Matrix4x4<T>(
          x, 0.F, 0.F, 0.F,
        0.F,   y, 0.F, 0.F,
        0.F, 0.F,   z, 0.F,
        0.F, 0.F, 0.F, 1.F
    );
    // clang-format on
}

template <typename T>
inline Matrix4x4<T> Scale(const Vector3<T>& v) {
    return Scale(v.x, v.y, v.z);
}

template <typename T>
inline Matrix4x4<T> Translate(T x, T y, T z) {
    // clang-format off
    return Matrix4x4<T>(
        1.F, 0.F, 0.F, 0.F,
        0.F, 1.F, 0.F, 0.F,
        0.F, 0.F, 1.F, 0.F,
          x,   y,   z, 1.F
    );
    // clang-format on
}

template <typename T>
inline Matrix4x4<T> Translate(const Vector3<T>& v) {
    return Translate(v.x, v.y, v.z);
}

}  // namespace math

}  // namespace engine
