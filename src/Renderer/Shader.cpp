#include <System/LogManager.hpp>
#include <System/StringFormat.hpp>

#include "Shader.hpp"

namespace engine {

namespace {

const String sTag("Shader");

UniformBufferObject::DataType GetUBODataTypeFromString(const String& str) {
    if (str == "mat4x4") return UniformBufferObject::DataType::MATRIX4X4;
    if (str == "mat3x3") return UniformBufferObject::DataType::MATRIX3X3;
    if (str == "mat2x2") return UniformBufferObject::DataType::MATRIX2X2;
    if (str == "vec4") return UniformBufferObject::DataType::VECTOR4;
    if (str == "vec3") return UniformBufferObject::DataType::VECTOR3;
    if (str == "vec2") return UniformBufferObject::DataType::VECTOR2;
    return UniformBufferObject::DataType::UNKNOWN;
}

}  // namespace

Shader::Shader() {}

Shader::~Shader() {}

UniformBufferObject& Shader::GetUBO() {
    return m_ubo;
}

void Shader::SetDescriptor(json&& descriptor) {
    m_descriptor = std::move(descriptor);

    std::vector<UniformBufferObject::Item> attributes;
    for (auto& attribute : m_descriptor["uniform_buffer"]["attributes"]) {
        String name = attribute["name"];
        String type = attribute["type"];
        attributes.push_back({name, GetUBODataTypeFromString(type)});
    }
    m_ubo.SetAttributes(attributes);
}

const json& Shader::GetDescriptor() {
    return m_descriptor;
}

}  // namespace engine
