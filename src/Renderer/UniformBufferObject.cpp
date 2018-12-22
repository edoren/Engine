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

void SetDataAtOffset(const void* data, size_t size, size_t offset, byte* buffer,
                     size_t buffer_size) {
    if (offset + size <= buffer_size) {
        std::memcpy(buffer + offset, data, size);
    } else {
        LogError(sTag, "Out of memory bounds");
    }
}

void* aligned_malloc(size_t size, size_t align) {
    void* ptr = nullptr;

    assert((align & (align - 1)) == 0);

    if (align > 0 && size > 0) {
        uint32 hdr_size = sizeof(uint16) + (static_cast<uint32>(align) - 1);
        void* p = malloc(size + hdr_size);

        if (p) {
            uintptr_t ptr_offset = (uintptr_t)p + sizeof(uint16);
            ptr = (void*)((ptr_offset + (align - 1)) & ~(align - 1));

            *((uint16*)ptr - 1) = (uint16)((uintptr_t)ptr - (uintptr_t)p);
        }
    }

    return ptr;
}

void aligned_free(void* ptr) {
    assert(ptr);
    uint16 offset = *((uint16*)ptr - 1);
    void* p = (void*)((uint8_t*)ptr - offset);
    free(p);
}

}  // namespace

UniformBufferObject::UniformBufferObject()
      : m_size(0),
        m_dynamic_alignment(0),
        m_layout_type(LayoutType::STD140),
        m_buffer(nullptr),
        m_buffer_size(0) {}

UniformBufferObject::UniformBufferObject(const std::vector<Item>& attributes)
      : UniformBufferObject() {
    SetAttributes(attributes);
}

UniformBufferObject::~UniformBufferObject() {
    if (m_dynamic_alignment > 0) {
        aligned_free(m_buffer);
    } else {
        free(m_buffer);
    }
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

    m_buffer = (byte*)malloc(current_allignment);
    m_size = current_allignment;
    m_buffer_size = m_size;
    m_attributes = attributes;
}

void UniformBufferObject::SetBufferSize(size_t num_ubo_instances,
                                        size_t min_ubo_alignment) {
    if (m_buffer != nullptr) {
        if (m_dynamic_alignment > 0) {
            aligned_free(m_buffer);
        } else {
            free(m_buffer);
        }
    }

    m_dynamic_alignment = GetSize();
    if (min_ubo_alignment > 0) {
        m_dynamic_alignment = (m_dynamic_alignment + min_ubo_alignment - 1) &
                              ~(min_ubo_alignment - 1);
    }
    size_t buffer_size = num_ubo_instances * m_dynamic_alignment;

    LogDebug(sTag, "UBO size: {} bytes"_format(GetSize()));
    LogDebug(sTag, "Minimum UBO alignment: {} bytes"_format(min_ubo_alignment));
    LogDebug(sTag,
             "Dynamic UBO alignment: {} bytes"_format(m_dynamic_alignment));

    m_buffer = (byte*)aligned_malloc(buffer_size, m_dynamic_alignment);
    m_buffer_size = buffer_size;
}

void UniformBufferObject::SetAttributeValue(const String& name,
                                            const math::mat4& value,
                                            size_t offset) {
    auto it = std::find(m_attributes.begin(), m_attributes.end(), name);
    if (it != m_attributes.end() && it->type == DataType::MATRIX4X4) {
        size_t pos = it - m_attributes.begin();
        size_t attribute_offset = m_attributes_alligned_offset[pos];

        if (m_layout_type == LayoutType::STD140) {
            for (size_t col = 0; col < 4; col++) {
                for (size_t row = 0; row < 4; row++) {
                    math::mat4::value_type val = value(row, col);
                    SetDataAtOffset(&val, sizeof(val), offset + attribute_offset,
                                    m_buffer, m_buffer_size);
                    attribute_offset += sizeof(val);
                }
            }
        }
    } else {
        LogError(sTag, "Not a valid UBO attribute name: {}"_format(name));
    }
}

void UniformBufferObject::SetAttributeValue(const String& name,
                                            const math::mat3& value,
                                            size_t offset) {
    auto it = std::find(m_attributes.begin(), m_attributes.end(), name);
    if (it != m_attributes.end() && it->type == DataType::MATRIX3X3) {
        size_t pos = it - m_attributes.begin();
        size_t attribute_offset = m_attributes_alligned_offset[pos];

        if (m_layout_type == LayoutType::STD140) {
            for (size_t col = 0; col < 3; col++) {
                for (size_t row = 0; row < 3; row++) {
                    math::mat3::value_type val = value(row, col);
                    SetDataAtOffset(&val, sizeof(math::mat3::value_type),
                                    offset + attribute_offset, m_buffer,
                                    m_buffer_size);
                    attribute_offset += sizeof(math::mat3::value_type);
                }
            }
        }
    } else {
        LogError(sTag, "Not a valid UBO attribute name: {}"_format(name));
    }
}

void UniformBufferObject::SetAttributeValue(const String& name,
                                            const math::mat2& value,
                                            size_t offset) {
    auto it = std::find(m_attributes.begin(), m_attributes.end(), name);
    if (it != m_attributes.end() && it->type == DataType::MATRIX2X2) {
        size_t pos = it - m_attributes.begin();
        size_t attribute_offset = m_attributes_alligned_offset[pos];

        if (m_layout_type == LayoutType::STD140) {
            for (size_t col = 0; col < 2; col++) {
                for (size_t row = 0; row < 2; row++) {
                    math::mat2::value_type val = value(row, col);
                    SetDataAtOffset(&val, sizeof(math::mat2::value_type),
                                    offset + attribute_offset, m_buffer,
                                    m_buffer_size);
                    attribute_offset += sizeof(math::mat2::value_type);
                }
            }
        }
    } else {
        LogError(sTag, "Not a valid UBO attribute name: {}"_format(name));
    }
}

void UniformBufferObject::SetAttributeValue(const String& name,
                                            const math::vec4& value,
                                            size_t offset) {
    auto it = std::find(m_attributes.begin(), m_attributes.end(), name);
    if (it != m_attributes.end() && it->type == DataType::VECTOR4) {
        size_t pos = it - m_attributes.begin();
        size_t attribute_offset = m_attributes_alligned_offset[pos];
        math::Vector4Packed<float> packed_vector(value);
        SetDataAtOffset(&packed_vector, sizeof(packed_vector),
                        offset + attribute_offset, m_buffer, m_buffer_size);
    } else {
        LogError(sTag, "Not a valid UBO attribute name: {}"_format(name));
    }
}

void UniformBufferObject::SetAttributeValue(const String& name,
                                            const math::vec3& value,
                                            size_t offset) {
    auto it = std::find(m_attributes.begin(), m_attributes.end(), name);
    if (it != m_attributes.end() && it->type == DataType::VECTOR3) {
        size_t pos = it - m_attributes.begin();
        size_t attribute_offset = m_attributes_alligned_offset[pos];
        math::Vector3Packed<float> packed_vector(value);
        SetDataAtOffset(&packed_vector, sizeof(packed_vector),
                        offset + attribute_offset, m_buffer, m_buffer_size);
    } else {
        LogError(sTag, "Not a valid UBO attribute name: {}"_format(name));
    }
}

void UniformBufferObject::SetAttributeValue(const String& name,
                                            const math::vec2& value,
                                            size_t offset) {
    auto it = std::find(m_attributes.begin(), m_attributes.end(), name);
    if (it != m_attributes.end() && it->type == DataType::VECTOR2) {
        size_t pos = it - m_attributes.begin();
        size_t attribute_offset = m_attributes_alligned_offset[pos];
        math::Vector2Packed<float> packed_vector(value);
        SetDataAtOffset(&packed_vector, sizeof(packed_vector),
                        offset + attribute_offset, m_buffer, m_buffer_size);
    } else {
        LogError(sTag, "Not a valid UBO attribute name: {}"_format(name));
    }
}

void UniformBufferObject::SetAttributeValue(const String& name,
                                            const void* value, size_t offset) {
    size_t buffer_offset = 0;
    auto it = std::find_if(m_attributes.begin(), m_attributes.end(),
                           [this, &name, &buffer_offset](Item& item) {
                               if (item.name == name) return true;
                               buffer_offset += GetTypeSize(item.type);
                               return false;
                           });

    if (it != m_attributes.end()) {
        size_t value_size = GetTypeSize(it->type);
        ENGINE_UNUSED(value);
        ENGINE_UNUSED(offset);
        ENGINE_UNUSED(value_size);
        LogError(sTag,
                 "SetAttributeValue(const String&, const void*) not "
                 "implemented yet");
    } else {
        LogError(sTag, "Not a valid UBO attribute name: {}"_format(name));
    }
}

size_t UniformBufferObject::GetSize() const {
    return m_size;
}

size_t UniformBufferObject::GetDynamicAlignment() const {
    return m_dynamic_alignment;
}

byte* UniformBufferObject::GetData() {
    return m_buffer;
}

const byte* UniformBufferObject::GetData() const {
    return m_buffer;
}

size_t UniformBufferObject::GetDataSize() const {
    return m_buffer_size != 0 ? m_buffer_size : GetSize();
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
