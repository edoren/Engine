#include <System/LogManager.hpp>
#include <System/StringFormat.hpp>
#include <System/StringView.hpp>
#include <Util/Container/Vector.hpp>

#include "Vk_VertexLayout.hpp"

namespace engine::plugin::vulkan {

namespace {

// const StringView sTag("Vk_VertexLayout");

Vector<VkVertexInputAttributeDescription> GetAttribDescription(const Vector<VertexLayout::Component>& input,
                                                               uint32 bufferBindId) {
    Vector<VkVertexInputAttributeDescription> attributeDescriptions;
    uint32 location = 0;
    uint32 bufferOffset = 0;
    for (const auto& component : input) {
        VkFormat format;
        uint32 size;
        switch (component) {
            case VertexLayout::Component::UV:
                format = VK_FORMAT_R32G32_SFLOAT;
                size = 2 * sizeof(float);
                break;
            case VertexLayout::Component::POSITION:
            case VertexLayout::Component::NORMAL:
                format = VK_FORMAT_R32G32B32_SFLOAT;
                size = 3 * sizeof(float);
                break;
            case VertexLayout::Component::COLOR:
                format = VK_FORMAT_R32G32B32A32_SFLOAT;
                size = 4 * sizeof(float);
                break;
            default:
                continue;
        }
        attributeDescriptions.push_back({
            .location = location++,
            .binding = bufferBindId,
            .format = format,
            .offset = bufferOffset,
        });
        bufferOffset += size;
    }
    return attributeDescriptions;
}

}  // namespace

Vk_VertexLayout::Vk_VertexLayout() = default;

Vk_VertexLayout::Vk_VertexLayout(const Vector<VertexLayout::Component>& components) : VertexLayout(components) {}

Vk_VertexLayout::Vk_VertexLayout(Vector<VertexLayout::Component>&& components) : VertexLayout(std::move(components)) {}

Vector<VkVertexInputAttributeDescription> Vk_VertexLayout::getVertexInputAttributeDescription(
    uint32 bufferBindId) const {
    return GetAttribDescription(m_vertexInput, bufferBindId);
}

}  // namespace engine::plugin::vulkan
