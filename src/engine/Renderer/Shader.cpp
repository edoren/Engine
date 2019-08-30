#include <System/LogManager.hpp>
#include <System/StringFormat.hpp>

#include "Shader.hpp"

namespace engine {

namespace {

const String sTag("Shader");

}  // namespace

Shader::Shader() {}

Shader::~Shader() {}

UniformBufferObject::DataType Shader::GetUBODataTypeFromString(
    const String& str) {
    if (str == "mat4x4") return UniformBufferObject::DataType::MATRIX4X4;
    if (str == "mat3x3") return UniformBufferObject::DataType::MATRIX3X3;
    if (str == "mat2x2") return UniformBufferObject::DataType::MATRIX2X2;
    if (str == "vec4") return UniformBufferObject::DataType::VECTOR4;
    if (str == "vec3") return UniformBufferObject::DataType::VECTOR3;
    if (str == "vec2") return UniformBufferObject::DataType::VECTOR2;
    return UniformBufferObject::DataType::UNKNOWN;
}

}  // namespace engine
