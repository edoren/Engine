#include <System/LogManager.hpp>
#include <System/StringFormat.hpp>

#include "UniformBufferObject.hpp"

namespace engine {

bool operator==(const String& lhs, const UniformBufferObject::Item& rhs) {
    return lhs == rhs.name;
}

bool operator==(const UniformBufferObject::Item& lhs, const String& rhs) {
    return rhs == lhs.name;
}

namespace {

const String sTag("UniformBufferObject");

void SetDataAtOffset(const void* data, size_t size, size_t offset,
                     std::vector<byte>& buffer) {
    if (offset + size <= buffer.size()) {
        std::memcpy(buffer.data() + offset, data, size);
    } else {
        LogError(sTag, "Out of memory bounds");
    }
}

}  // namespace

UniformBufferObject::UniformBufferObject()
      : m_layout_type(LayoutType::STD140) {}

UniformBufferObject::UniformBufferObject(const std::vector<Item>& attributes) {
    SetAttributes(attributes);
}

void UniformBufferObject::SetAttributes(const std::vector<Item>& attributes) {
    m_attributes_alligned_offset.clear();

    size_t current_allignment = 0;

    for (auto& item : attributes) {
        size_t attr_size = GetTypeSize(item.type);
        size_t attr_req_allignment = GetTypeAllignment(item.type);

        // Find the next alligned block
        size_t padding = attr_req_allignment -
                         (current_allignment & (attr_req_allignment - 1));
        if (padding != attr_req_allignment) {
            current_allignment += padding;
        }

        m_attributes_alligned_offset.push_back(current_allignment);
        current_allignment += attr_size;
    }

    m_buffer.assign(current_allignment, 0);
    m_attributes = attributes;
}

void UniformBufferObject::SetAttributeValue(const String& name,
                                            const math::mat4& value) {
    auto it = std::find(m_attributes.begin(), m_attributes.end(), name);
    if (it != m_attributes.end()) {
        size_t pos = it - m_attributes.begin();
        size_t offset = m_attributes_alligned_offset[pos];

        if (m_layout_type == LayoutType::STD140) {
            for (size_t col = 0; col < 4; col++) {
                for (size_t row = 0; row < 4; row++) {
                    math::mat4::value_type val = value(row, col);
                    SetDataAtOffset(
                        &val, sizeof(math::mat4::value_type),
                        offset + (row * sizeof(math::mat4::value_type)),
                        m_buffer);
                }
                offset += 16;
            }
        }
    } else {
        LogError(sTag, "Not a valid UBO attribute name: {}"_format(name));
    }
}

void UniformBufferObject::SetAttributeValue(const String& name,
                                            const math::mat3& value) {
    auto it = std::find(m_attributes.begin(), m_attributes.end(), name);
    if (it != m_attributes.end()) {
        size_t pos = it - m_attributes.begin();
        size_t offset = m_attributes_alligned_offset[pos];

        if (m_layout_type == LayoutType::STD140) {
            for (size_t col = 0; col < 4; col++) {
                for (size_t row = 0; row < 4; row++) {
                    math::mat3::value_type val = value(row, col);
                    SetDataAtOffset(
                        &val, sizeof(math::mat3::value_type),
                        offset + (row * sizeof(math::mat3::value_type)),
                        m_buffer);
                }
                offset += 16;
            }
        }
    } else {
        LogError(sTag, "Not a valid UBO attribute name: {}"_format(name));
    }
}

void UniformBufferObject::SetAttributeValue(const String& name,
                                            const math::mat2& value) {
    auto it = std::find(m_attributes.begin(), m_attributes.end(), name);
    if (it != m_attributes.end()) {
        size_t pos = it - m_attributes.begin();
        size_t offset = m_attributes_alligned_offset[pos];

        if (m_layout_type == LayoutType::STD140) {
            for (size_t col = 0; col < 4; col++) {
                for (size_t row = 0; row < 4; row++) {
                    math::mat2::value_type val = value(row, col);
                    SetDataAtOffset(
                        &val, sizeof(math::mat2::value_type),
                        offset + (row * sizeof(math::mat2::value_type)),
                        m_buffer);
                }
                offset += 16;
            }
        }
    } else {
        LogError(sTag, "Not a valid UBO attribute name: {}"_format(name));
    }
}

void UniformBufferObject::SetAttributeValue(const String& name,
                                            const math::vec4& value) {
    auto it = std::find(m_attributes.begin(), m_attributes.end(), name);
    if (it != m_attributes.end()) {
        size_t pos = it - m_attributes.begin();
        size_t offset = m_attributes_alligned_offset[pos];
        math::Vector4Packed<float> packed_vector(value);
        SetDataAtOffset(&packed_vector, sizeof(packed_vector), offset,
                        m_buffer);
    } else {
        LogError(sTag, "Not a valid UBO attribute name: {}"_format(name));
    }
}

void UniformBufferObject::SetAttributeValue(const String& name,
                                            const math::vec3& value) {
    auto it = std::find(m_attributes.begin(), m_attributes.end(), name);
    if (it != m_attributes.end()) {
        size_t pos = it - m_attributes.begin();
        size_t offset = m_attributes_alligned_offset[pos];
        math::Vector3Packed<float> packed_vector(value);
        SetDataAtOffset(&packed_vector, sizeof(packed_vector), offset,
                        m_buffer);
    } else {
        LogError(sTag, "Not a valid UBO attribute name: {}"_format(name));
    }
}

void UniformBufferObject::SetAttributeValue(const String& name,
                                            const math::vec2& value) {
    auto it = std::find(m_attributes.begin(), m_attributes.end(), name);
    if (it != m_attributes.end()) {
        size_t pos = it - m_attributes.begin();
        size_t offset = m_attributes_alligned_offset[pos];
        math::Vector2Packed<float> packed_vector(value);
        SetDataAtOffset(&packed_vector, sizeof(packed_vector), offset,
                        m_buffer);
    } else {
        LogError(sTag, "Not a valid UBO attribute name: {}"_format(name));
    }
}

void UniformBufferObject::SetAttributeValue(const String& name,
                                            const void* value) {
    size_t buffer_offset = 0;
    auto it = std::find_if(m_attributes.begin(), m_attributes.end(),
                           [this, &name, &buffer_offset](Item& item) {
                               if (item.name == name) return true;
                               buffer_offset += GetTypeSize(item.type);
                               return false;
                           });

    if (it != m_attributes.end()) {
        size_t value_size = GetTypeSize(it->type);

    } else {
        LogError(sTag, "Not a valid UBO attribute name: {}"_format(name));
    }
}

byte* UniformBufferObject::GetData() {
    return m_buffer.data();
}

const byte* UniformBufferObject::GetData() const {
    return m_buffer.data();
}

size_t UniformBufferObject::GetDataSize() const {
    return m_buffer.size();
}

size_t UniformBufferObject::GetTypeSize(DataType type) {
    switch (m_layout_type) {
        case LayoutType::STD140: {
            switch (type) {
                case DataType::MATRIX4X4:
                    return 64;
                case DataType::MATRIX3X3:
                    return 64;
                case DataType::MATRIX2X2:
                    return 64;
                case DataType::VECTOR4:
                    return 16;
                case DataType::VECTOR3:
                    return 16;
                case DataType::VECTOR2:
                    return 8;
                case DataType::UNKNOWN:
                default:
                    return 0;
            }
        }
        case LayoutType::STD430: {
            return 0;
        }
    }
    return 0;
}

size_t UniformBufferObject::GetTypeAllignment(DataType type) {
    switch (m_layout_type) {
        case LayoutType::STD140: {
            switch (type) {
                case DataType::MATRIX4X4:
                    return 16;
                case DataType::MATRIX3X3:
                    return 16;
                case DataType::MATRIX2X2:
                    return 16;
                case DataType::VECTOR4:
                    return 16;
                case DataType::VECTOR3:
                    return 16;
                case DataType::VECTOR2:
                    return 8;
                case DataType::UNKNOWN:
                default:
                    return 0;
            }
        }
        case LayoutType::STD430: {
            return 0;
        }
    }
    return 0;
}

}  // namespace engine
