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

using bvec2 = Vector2<bool>;
using bvec3 = Vector3<bool>;
using bvec4 = Vector4<bool>;

using ivec2 = Vector2<int32>;
using ivec3 = Vector3<int32>;
using ivec4 = Vector4<int32>;

using uvec2 = Vector2<uint32>;
using uvec3 = Vector3<uint32>;
using uvec4 = Vector4<uint32>;

using vec2 = Vector2<float>;
using vec3 = Vector3<float>;
using vec4 = Vector4<float>;

using dvec2 = Vector2<double>;
using dvec3 = Vector3<double>;
using dvec4 = Vector4<double>;

using mat2 = Matrix2x2<float>;
using mat3 = Matrix3x3<float>;
using mat4 = Matrix4x4<float>;

using mat2x2 = Matrix2x2<float>;
using mat3x3 = Matrix3x3<float>;
using mat4x4 = Matrix4x4<float>;

}  // namespace math

}  // namespace engine
