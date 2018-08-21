#pragma once

#include <Util/Prerequisites.hpp>

#include <Math/Math.hpp>
#include <System/String.hpp>

namespace engine {

class ENGINE_API UniformBufferObject {
    friend class Shader;

public:
    enum class DataType {
        MATRIX4X4,
        MATRIX3X3,
        MATRIX2X2,
        VECTOR4,
        VECTOR3,
        VECTOR2,
        UNKNOWN
    };

    enum class LayoutType {
        STD140,
        STD430  // Not supported currently
    };

    struct Item {
        String name;  // TODO: maybe replace this for an enum?
        DataType type;
    };

    void SetAttributes(const std::vector<Item>& attributes);

    void SetAttributeValue(const String& name, const math::mat4& value);
    void SetAttributeValue(const String& name, const math::mat3& value);
    void SetAttributeValue(const String& name, const math::mat2& value);
    void SetAttributeValue(const String& name, const math::vec4& value);
    void SetAttributeValue(const String& name, const math::vec3& value);
    void SetAttributeValue(const String& name, const math::vec2& value);

    byte* GetData();

    const byte* GetData() const;

    size_t GetDataSize() const;

private:
    UniformBufferObject();

    explicit UniformBufferObject(const std::vector<Item>& attributes);

    void SetAttributeValue(const String& name, const void* value);

    size_t GetTypeSize(UniformBufferObject::DataType type);
    size_t GetTypeAllignment(UniformBufferObject::DataType type);

private:
    LayoutType m_layout_type;
    std::vector<byte> m_buffer;
    std::vector<Item> m_attributes;
    std::vector<size_t> m_attributes_alligned_offset;
};

}  // namespace engine
