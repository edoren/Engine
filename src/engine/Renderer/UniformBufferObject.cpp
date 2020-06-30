#include <Renderer/UniformBufferObject.hpp>

#include <System/LogManager.hpp>
#include <System/StringFormat.hpp>
#include <System/StringView.hpp>
#include <Util/Container/Vector.hpp>

namespace engine {

bool operator==(const String& lhs, const UniformBufferObject::Item& rhs) {
    return lhs == rhs.name;
}

bool operator==(const UniformBufferObject::Item& lhs, const String& rhs) {
    return rhs == lhs.name;
}

namespace {

const StringView sTag("UniformBufferObject");

constexpr bool is_power_of_two(size_t num) {
    return (num & (num - 1)) == 0;
}

void* aligned_malloc(size_t size, size_t align) {
    void* ptr = nullptr;

    assert(is_power_of_two(align));

    if (align > 0 && size > 0) {
        uint32 hdrSize = sizeof(uint16) + (static_cast<uint32>(align) - 1);
        void* p = malloc(size + hdrSize);

        if (p) {
            uintptr_t ptrOffset = (uintptr_t)p + sizeof(uint16);
            ptr = (void*)((ptrOffset + (align - 1)) & ~(align - 1));

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
        m_dynamicAlignment(0),
        m_layoutType(LayoutType::STD140),
        m_buffer(nullptr),
        m_bufferSize(0),
        m_bufferChanged(false) {}

UniformBufferObject::UniformBufferObject(const Vector<Item>& attributes) : UniformBufferObject() {
    setAttributes(attributes);
}

UniformBufferObject::~UniformBufferObject() {
    if (m_dynamicAlignment > 0) {
        aligned_free(m_buffer);
    } else {
        free(m_buffer);
    }
}

void UniformBufferObject::setAttributes(const Vector<Item>& attributes) {
    m_attributesAllignedOffset.clear();

    size_t currentAllignment = 0;

    for (const auto& item : attributes) {
        size_t attrSize = getTypeSize(item.type);
        size_t attrReqAllignment = getTypeAllignment(item.type);

        // Find the next alligned block
        size_t padding = attrReqAllignment - (currentAllignment & (attrReqAllignment - 1));
        if (padding != attrReqAllignment) {
            currentAllignment += padding;
        }

        m_attributesAllignedOffset.push_back(currentAllignment);
        currentAllignment += attrSize;
    }

    m_buffer = (byte*)malloc(currentAllignment);
    m_size = currentAllignment;
    m_bufferSize = m_size;
    m_attributes = attributes;
}

void UniformBufferObject::setBufferSize(size_t numUboInstances, size_t minUboAlignment) {
    if (m_buffer != nullptr) {
        if (m_dynamicAlignment > 0) {
            aligned_free(m_buffer);
        } else {
            free(m_buffer);
        }
    }

    m_dynamicAlignment = getSize();

    // Get the next power of 2
    size_t i;
    size_t number = m_dynamicAlignment;
    for (i = 0; number > 1; i++) {
        number = number >> 1;
    }
    m_dynamicAlignment = size_t(1) << (i + 1);

    // Calculate the alignment based on the min required alignment
    if (minUboAlignment > getSize() && is_power_of_two(minUboAlignment)) {
        m_dynamicAlignment = minUboAlignment;
    } else if (minUboAlignment > 0) {
        m_dynamicAlignment = (m_dynamicAlignment + minUboAlignment - 1) & ~(minUboAlignment - 1);
    }

    size_t bufferSize = numUboInstances * m_dynamicAlignment;

    LogDebug(sTag, "UBO size: {} bytes", getSize());
    LogDebug(sTag, "Minimum UBO alignment: {} bytes", minUboAlignment);
    LogDebug(sTag, "Dynamic UBO alignment: {} bytes", m_dynamicAlignment);

    m_buffer = (byte*)aligned_malloc(bufferSize, m_dynamicAlignment);
    m_bufferSize = bufferSize;
}

void UniformBufferObject::setAttributeValue(const String& name, const math::mat4& value, size_t offset) {
    auto it = std::find(m_attributes.begin(), m_attributes.end(), name);
    if (it != m_attributes.end() && it->type == DataType::MATRIX4X4) {
        size_t pos = it - m_attributes.begin();
        size_t attributeOffset = m_attributesAllignedOffset[pos];

        if (m_layoutType == LayoutType::STD140) {
            for (size_t col = 0; col < 4; col++) {
                for (size_t row = 0; row < 4; row++) {
                    math::mat4::value_type val = value(row, col);
                    setDataAtOffset(&val, sizeof(val), offset + attributeOffset);
                    attributeOffset += sizeof(val);
                }
            }
        }
    } else {
        LogError(sTag, "Not a valid UBO attribute name: {}", name);
    }
}

void UniformBufferObject::setAttributeValue(const String& name, const math::mat3& value, size_t offset) {
    auto it = std::find(m_attributes.begin(), m_attributes.end(), name);
    if (it != m_attributes.end() && it->type == DataType::MATRIX3X3) {
        size_t pos = it - m_attributes.begin();
        size_t attributeOffset = m_attributesAllignedOffset[pos];

        if (m_layoutType == LayoutType::STD140) {
            for (size_t col = 0; col < 3; col++) {
                for (size_t row = 0; row < 3; row++) {
                    math::mat3::value_type val = value(row, col);
                    setDataAtOffset(&val, sizeof(math::mat3::value_type), offset + attributeOffset);
                    attributeOffset += sizeof(math::mat3::value_type);
                }
            }
        }
    } else {
        LogError(sTag, "Not a valid UBO attribute name: {}", name);
    }
}

void UniformBufferObject::setAttributeValue(const String& name, const math::mat2& value, size_t offset) {
    auto it = std::find(m_attributes.begin(), m_attributes.end(), name);
    if (it != m_attributes.end() && it->type == DataType::MATRIX2X2) {
        size_t pos = it - m_attributes.begin();
        size_t attributeOffset = m_attributesAllignedOffset[pos];

        if (m_layoutType == LayoutType::STD140) {
            for (size_t col = 0; col < 2; col++) {
                for (size_t row = 0; row < 2; row++) {
                    math::mat2::value_type val = value(row, col);
                    setDataAtOffset(&val, sizeof(math::mat2::value_type), offset + attributeOffset);
                    attributeOffset += sizeof(math::mat2::value_type);
                }
            }
        }
    } else {
        LogError(sTag, "Not a valid UBO attribute name: {}", name);
    }
}

void UniformBufferObject::setAttributeValue(const String& name, const math::vec4& value, size_t offset) {
    auto it = std::find(m_attributes.begin(), m_attributes.end(), name);
    if (it != m_attributes.end() && it->type == DataType::VECTOR4) {
        size_t pos = it - m_attributes.begin();
        size_t attributeOffset = m_attributesAllignedOffset[pos];
        math::Vector4Packed<float> packedVector(value);
        setDataAtOffset(&packedVector, sizeof(packedVector), offset + attributeOffset);
    } else {
        LogError(sTag, "Not a valid UBO attribute name: {}", name);
    }
}

void UniformBufferObject::setAttributeValue(const String& name, const math::vec3& value, size_t offset) {
    auto it = std::find(m_attributes.begin(), m_attributes.end(), name);
    if (it != m_attributes.end() && it->type == DataType::VECTOR3) {
        size_t pos = it - m_attributes.begin();
        size_t attributeOffset = m_attributesAllignedOffset[pos];
        math::Vector3Packed<float> packedVector(value);
        setDataAtOffset(&packedVector, sizeof(packedVector), offset + attributeOffset);
    } else {
        LogError(sTag, "Not a valid UBO attribute name: {}", name);
    }
}

void UniformBufferObject::setAttributeValue(const String& name, const math::vec2& value, size_t offset) {
    auto it = std::find(m_attributes.begin(), m_attributes.end(), name);
    if (it != m_attributes.end() && it->type == DataType::VECTOR2) {
        size_t pos = it - m_attributes.begin();
        size_t attributeOffset = m_attributesAllignedOffset[pos];
        math::Vector2Packed<float> packedVector(value);
        setDataAtOffset(&packedVector, sizeof(packedVector), offset + attributeOffset);
    } else {
        LogError(sTag, "Not a valid UBO attribute name: {}", name);
    }
}

void UniformBufferObject::setAttributeValue(const String& name, const void* value, size_t offset) {
    size_t bufferOffset = 0;
    auto it = std::find_if(m_attributes.begin(), m_attributes.end(), [this, &name, &bufferOffset](Item& item) {
        if (item.name == name) {
            return true;
        }
        bufferOffset += getTypeSize(item.type);
        return false;
    });

    if (it != m_attributes.end()) {
        size_t valueSize = getTypeSize(it->type);
        ENGINE_UNUSED(value);
        ENGINE_UNUSED(offset);
        ENGINE_UNUSED(valueSize);
        LogError(sTag, "SetAttributeValue(const String&, const void*) not implemented yet");
    } else {
        LogError(sTag, "Not a valid UBO attribute name: {}", name);
    }
}

void UniformBufferObject::setDataAtOffset(const void* data, size_t size, size_t offset) {
    if (offset + size <= m_bufferSize) {
        std::memcpy(m_buffer + offset, data, size);
        m_bufferChanged = true;
    } else {
        LogError(sTag, "Out of memory bounds");
    }
}

size_t UniformBufferObject::getSize() const {
    return m_size;
}

size_t UniformBufferObject::getDynamicAlignment() const {
    return m_dynamicAlignment;
}

byte* UniformBufferObject::getData() {
    return m_buffer;
}

const byte* UniformBufferObject::getData() const {
    return m_buffer;
}

size_t UniformBufferObject::getDataSize() const {
    return m_bufferSize != 0 ? m_bufferSize : getSize();
}

size_t UniformBufferObject::getTypeSize(DataType type) {
    switch (m_layoutType) {
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

size_t UniformBufferObject::getTypeAllignment(DataType type) {
    switch (m_layoutType) {
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
