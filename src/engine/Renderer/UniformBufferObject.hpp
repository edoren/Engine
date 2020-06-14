#pragma once

#include <Util/Prerequisites.hpp>

#include <Math/Math.hpp>
#include <System/String.hpp>

namespace engine {

class ENGINE_API UniformBufferObject {
public:
    UniformBufferObject();
    ~UniformBufferObject();

    enum class DataType {
        MATRIX4X4,
        MATRIX3X3,
        MATRIX2X2,
        VECTOR4,
        VECTOR3,
        VECTOR2,
        UNKNOWN,
    };

    enum class LayoutType {
        STD140,
        STD430  // Not supported currently
    };

    struct Item {
        String name;  // TODO: maybe replace this for an enum?
        DataType type;
    };

    void setBufferSize(size_t num_ubo_instances, size_t min_ubo_alignment);

    void setAttributes(const std::vector<Item>& attributes);

    void setAttributeValue(const String& name, const math::mat4& value, size_t offset = 0);
    void setAttributeValue(const String& name, const math::mat3& value, size_t offset = 0);
    void setAttributeValue(const String& name, const math::mat2& value, size_t offset = 0);
    void setAttributeValue(const String& name, const math::vec4& value, size_t offset = 0);
    void setAttributeValue(const String& name, const math::vec3& value, size_t offset = 0);
    void setAttributeValue(const String& name, const math::vec2& value, size_t offset = 0);

    size_t getSize() const;
    size_t getDynamicAlignment() const;

    byte* getData();
    const byte* getData() const;
    size_t getDataSize() const;

private:
    explicit UniformBufferObject(const std::vector<Item>& attributes);

    void setAttributeValue(const String& name, const void* value, size_t offset = 0);

    void setDataAtOffset(const void* data, size_t size, size_t offset);

    size_t getTypeSize(UniformBufferObject::DataType type);
    size_t getTypeAllignment(UniformBufferObject::DataType type);

    size_t m_size;
    size_t m_dynamicAlignment;

    LayoutType m_layoutType;

    byte* m_buffer;
    size_t m_bufferSize;
    bool m_bufferChanged;

    std::vector<Item> m_attributes;
    std::vector<size_t> m_attributesAllignedOffset;
};

}  // namespace engine
