#pragma once

#include <Util/Prerequisites.hpp>

#include "Vk_Config.hpp"
#include "Vk_Dependencies.hpp"

namespace engine {

class VULKAN_PLUGIN_API Vk_RenderResource {
public:
    Vk_RenderResource();
    ~Vk_RenderResource();

    bool create();
    void destroy();

    VkFramebuffer framebuffer;
    VkCommandBuffer commandBuffer;
    VkSemaphore imageAvailableSemaphore;
    VkSemaphore finishedRenderingSemaphore;
    VkFence fence;
};

}  // namespace engine
