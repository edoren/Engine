#include "Shader.hpp"

#include <System/LogManager.hpp>
#include <System/StringFormat.hpp>
#include <System/StringView.hpp>

namespace engine {

namespace {

// const StringView sTag("Shader");

}  // namespace

Shader::Shader() = default;

Shader::~Shader() = default;

UniformBufferObject::DataType Shader::getUboDataTypeFromString(const String& str) {
    if (str == "mat4x4") {
        return UniformBufferObject::DataType::MATRIX4X4;
    }
    if (str == "mat3x3") {
        return UniformBufferObject::DataType::MATRIX3X3;
    }
    if (str == "mat2x2") {
        return UniformBufferObject::DataType::MATRIX2X2;
    }
    if (str == "vec4") {
        return UniformBufferObject::DataType::VECTOR4;
    }
    if (str == "vec3") {
        return UniformBufferObject::DataType::VECTOR3;
    }
    if (str == "vec2") {
        return UniformBufferObject::DataType::VECTOR2;
    }
    return UniformBufferObject::DataType::UNKNOWN;
}

}  // namespace engine
