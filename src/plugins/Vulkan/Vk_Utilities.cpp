#include "Vk_Utilities.hpp"

#include <System/LogManager.hpp>
#include <System/String.hpp>

#include "Vk_Context.hpp"

namespace engine {

namespace {

const String sTag("Vk_Utilities");

}  // namespace

bool Vk_Utilities::AllocateBufferMemory(
    const VkBuffer& buffer, VkDeviceMemory* memory,
    const VkMemoryPropertyFlags& memory_properties) {
    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.GetVulkanDevice();

    VkMemoryRequirements buffer_memory_requirements;
    vkGetBufferMemoryRequirements(device, buffer, &buffer_memory_requirements);

    return Vk_Utilities::AllocateMemory(memory, memory_properties,
                                        buffer_memory_requirements);
}

bool Vk_Utilities::AllocateMemory(
    VkDeviceMemory* memory, const VkMemoryPropertyFlags& memory_properties,
    const VkMemoryRequirements& memory_requirements) {
    VkResult result = VK_SUCCESS;

    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.GetVulkanDevice();
    VkPhysicalDevice physical_device = context.GetPhysicalDevice();

    VkPhysicalDeviceMemoryProperties physical_memory_properties;
    vkGetPhysicalDeviceMemoryProperties(physical_device,
                                        &physical_memory_properties);

    for (uint32_t i = 0; i < physical_memory_properties.memoryTypeCount; i++) {
        if ((memory_requirements.memoryTypeBits & (1 << i)) &&
            (physical_memory_properties.memoryTypes[i].propertyFlags &
             memory_properties)) {
            VkMemoryAllocateInfo memory_allocate_info = {
                VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,  // sType
                nullptr,                                 // pNext
                memory_requirements.size,                // allocationSize
                i                                        // memoryTypeIndex
            };
            result = vkAllocateMemory(device, &memory_allocate_info, nullptr,
                                      memory);
            if (result == VK_SUCCESS) {
                return true;
            }
        }
    }

    return false;
}

bool Vk_Utilities::AllocateCommandBuffers(VkCommandPool& cmd_pool,
                                          uint32_t count,
                                          VkCommandBuffer* command_buffer) {
    VkResult result = VK_SUCCESS;

    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.GetVulkanDevice();

    // Allocate space in the pool for the buffer
    VkCommandBufferAllocateInfo cmd_buffer_allocate_info = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,  // sType
        nullptr,                                         // pNext
        cmd_pool,                                        // commandPool
        VK_COMMAND_BUFFER_LEVEL_PRIMARY,                 // level
        count                                            // bufferCount
    };
    result = vkAllocateCommandBuffers(device, &cmd_buffer_allocate_info,
                                      command_buffer);
    if (result != VK_SUCCESS) {
        LogError(sTag, "Could not allocate command buffer");
        return false;
    }

    return true;
}

bool Vk_Utilities::CreateVulkanSemaphore(VkSemaphore* semaphore) {
    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.GetVulkanDevice();

    VkSemaphoreCreateInfo semaphore_create_info = {
        VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,  // sType
        nullptr,                                  // pNext
        VkSemaphoreCreateFlags()                  // flags
    };

    VkResult result =
        vkCreateSemaphore(device, &semaphore_create_info, nullptr, semaphore);
    if (result != VK_SUCCESS) {
        LogError(sTag, "Could not create semaphore");
        return false;
    }

    return true;
}

bool Vk_Utilities::CreateVulkanFence(VkFenceCreateFlags flags, VkFence* fence) {
    VkResult result = VK_SUCCESS;

    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.GetVulkanDevice();

    VkFenceCreateInfo fence_create_info = {
        VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,  // sType
        nullptr,                              // pNext
        flags                                 // flags
    };

    result = vkCreateFence(device, &fence_create_info, nullptr, fence);
    if (result != VK_SUCCESS) {
        LogError(sTag, "Could not create fence");
        return false;
    }

    return true;
}

}  // namespace engine
