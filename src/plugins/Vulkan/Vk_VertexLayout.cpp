#include <System/LogManager.hpp>
#include <System/StringFormat.hpp>

#include "Vk_VertexLayout.hpp"

namespace engine {

namespace {

const String sTag("Vk_VertexLayout");

std::vector<VkVertexInputAttributeDescription> GetAttribDescription(const std::vector<VertexLayout::Component>& input,
                                                                    uint32 buffer_bind_id) {
    std::vector<VkVertexInputAttributeDescription> attribute_descriptions;
    uint32 location = 0;
    uint32 buffer_offset = 0;
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
                break;
        }
        attribute_descriptions.push_back({
            location++,      // location
            buffer_bind_id,  // binding
            format,          // format
            buffer_offset    // offset
        });
        buffer_offset += size;
    }
    return attribute_descriptions;
}

}  // namespace

Vk_VertexLayout::Vk_VertexLayout() {}

Vk_VertexLayout::Vk_VertexLayout(const std::vector<VertexLayout::Component>& components) : VertexLayout(components) {}

Vk_VertexLayout::Vk_VertexLayout(std::vector<VertexLayout::Component>&& components)
      : VertexLayout(std::move(components)) {}

std::vector<VkVertexInputAttributeDescription> Vk_VertexLayout::getVertexInputAttributeDescription(
    uint32 buffer_bind_id) const {
    return GetAttribDescription(m_vertex_input, buffer_bind_id);
}

}  // namespace engine
