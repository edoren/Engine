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
    VkCommandBuffer command_buffer;
    VkSemaphore image_available_semaphore;
    VkSemaphore finished_rendering_semaphore;
    VkFence fence;
};

}  // namespace engine
