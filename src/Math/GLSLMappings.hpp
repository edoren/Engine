#pragma once

#include <Util/Types.hpp>

#include <Math/Vector2.hpp>
#include <Math/Vector3.hpp>
#include <Math/Vector4.hpp>

#include <Math/Matrix2x2.hpp>
#include <Math/Matrix3x3.hpp>
#include <Math/Matrix4x4.hpp>

namespace engine {

namespace math {

typedef Vector2<bool> bvec2;
typedef Vector3<bool> bvec3;
typedef Vector4<bool> bvec4;

typedef Vector2<int32> ivec2;
typedef Vector3<int32> ivec3;
typedef Vector4<int32> ivec4;

typedef Vector2<uint32> uvec2;
typedef Vector3<uint32> uvec3;
typedef Vector4<uint32> uvec4;

typedef Vector2<float> vec2;
typedef Vector3<float> vec3;
typedef Vector4<float> vec4;

typedef Vector2<double> dvec2;
typedef Vector3<double> dvec3;
typedef Vector4<double> dvec4;

typedef Matrix2x2<float> mat2;
typedef Matrix3x3<float> mat3;
typedef Matrix4x4<float> mat4;

typedef Matrix2x2<float> mat2x2;
typedef Matrix3x3<float> mat3x3;
typedef Matrix4x4<float> mat4x4;

}  // namespace math

}  // namespace engine
