#pragma once

#include <Renderer/VertexLayout.hpp>
#include <Util/Container/Vector.hpp>

#include "Vk_Config.hpp"
#include "Vk_Dependencies.hpp"

namespace engine {

class VULKAN_PLUGIN_API Vk_VertexLayout : public VertexLayout {
public:
    Vk_VertexLayout();
    Vk_VertexLayout(const Vector<Component>& components);
    Vk_VertexLayout(Vector<Component>&& components);

    Vector<VkVertexInputAttributeDescription> getVertexInputAttributeDescription(uint32 bufferBindId) const;
};

}  // namespace engine
