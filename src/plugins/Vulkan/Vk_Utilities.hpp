#pragma once

#include "Vk_Config.hpp"
#include "Vk_Dependencies.hpp"

namespace engine {

class VULKAN_PLUGIN_API Vk_Utilities {
public:
    static bool AllocateBufferMemory(const VkBuffer& buffer,
                                     VkDeviceMemory* memory,
                                     const VkMemoryPropertyFlags& memoryProperties);
    static bool AllocateMemory(VkDeviceMemory* memory,
                               const VkMemoryPropertyFlags& memoryProperties,
                               const VkMemoryRequirements& memoryRequirements);
    static bool AllocateCommandBuffers(VkCommandPool& cmdPool, uint32_t count, VkCommandBuffer* commandBuffer);

    static bool CreateVulkanSemaphore(VkSemaphore* semaphore);
    static bool CreateVulkanFence(VkFenceCreateFlags flags, VkFence* fence);
};

}  // namespace engine
