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

    void SetBufferSize(size_t num_ubo_instances, size_t min_ubo_alignment);

    void SetAttributes(const std::vector<Item>& attributes);

    void SetAttributeValue(const String& name, const math::mat4& value, size_t offset = 0);
    void SetAttributeValue(const String& name, const math::mat3& value, size_t offset = 0);
    void SetAttributeValue(const String& name, const math::mat2& value, size_t offset = 0);
    void SetAttributeValue(const String& name, const math::vec4& value, size_t offset = 0);
    void SetAttributeValue(const String& name, const math::vec3& value, size_t offset = 0);
    void SetAttributeValue(const String& name, const math::vec2& value, size_t offset = 0);

    size_t GetSize() const;
    size_t GetDynamicAlignment() const;

    byte* GetData();
    const byte* GetData() const;
    size_t GetDataSize() const;

private:
    explicit UniformBufferObject(const std::vector<Item>& attributes);

    void SetAttributeValue(const String& name, const void* value, size_t offset = 0);

    void SetDataAtOffset(const void* data, size_t size, size_t offset);

    size_t GetTypeSize(UniformBufferObject::DataType type);
    size_t GetTypeAllignment(UniformBufferObject::DataType type);

private:
    size_t m_size;
    size_t m_dynamic_alignment;

    LayoutType m_layout_type;

    byte* m_buffer;
    size_t m_buffer_size;
    bool m_buffer_changed;

    std::vector<Item> m_attributes;
    std::vector<size_t> m_attributes_alligned_offset;
};

}  // namespace engine
