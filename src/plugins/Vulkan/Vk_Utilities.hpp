#pragma once

#include "Vk_Config.hpp"
#include "Vk_Dependencies.hpp"

namespace engine {

class VULKAN_PLUGIN_API Vk_Utilities {
public:
    static bool AllocateBufferMemory(
        const VkBuffer& buffer, VkDeviceMemory* memory,
        const VkMemoryPropertyFlags& memory_properties);
    static bool AllocateMemory(VkDeviceMemory* memory,
                               const VkMemoryPropertyFlags& memory_properties,
                               const VkMemoryRequirements& memory_requirements);
    static bool AllocateCommandBuffers(VkCommandPool& cmd_pool, uint32_t count,
                                       VkCommandBuffer* command_buffer);

    static bool CreateVulkanSemaphore(VkSemaphore* semaphore);
    static bool CreateVulkanFence(VkFenceCreateFlags flags, VkFence* fence);
};

}  // namespace engine
