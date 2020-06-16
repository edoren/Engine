#pragma once

#include <Renderer/VertexLayout.hpp>

#include "Vk_Config.hpp"
#include "Vk_Dependencies.hpp"

namespace engine {

class VULKAN_PLUGIN_API Vk_VertexLayout : public VertexLayout {
public:
    Vk_VertexLayout();
    Vk_VertexLayout(const std::vector<Component>& components);
    Vk_VertexLayout(std::vector<Component>&& components);

    std::vector<VkVertexInputAttributeDescription> getVertexInputAttributeDescription(uint32 bufferBindId) const;
};

}  // namespace engine
