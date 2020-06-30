#include "Vk_Utilities.hpp"

#include <System/LogManager.hpp>
#include <System/String.hpp>
#include <System/StringView.hpp>

#include "Vk_Context.hpp"

namespace engine {

namespace {

const StringView sTag("Vk_Utilities");

}  // namespace

bool Vk_Utilities::AllocateBufferMemory(const VkBuffer& buffer,
                                        VkDeviceMemory* memory,
                                        const VkMemoryPropertyFlags& memoryProperties) {
    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.getVulkanDevice();

    VkMemoryRequirements bufferMemoryRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &bufferMemoryRequirements);

    return Vk_Utilities::AllocateMemory(memory, memoryProperties, bufferMemoryRequirements);
}

bool Vk_Utilities::AllocateMemory(VkDeviceMemory* memory,
                                  const VkMemoryPropertyFlags& memoryProperties,
                                  const VkMemoryRequirements& memoryRequirements) {
    VkResult result = VK_SUCCESS;

    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.getVulkanDevice();
    VkPhysicalDevice physicalDevice = context.getPhysicalDevice();

    VkPhysicalDeviceMemoryProperties physicalMemoryProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &physicalMemoryProperties);

    for (uint32_t i = 0; i < physicalMemoryProperties.memoryTypeCount; i++) {
        if ((memoryRequirements.memoryTypeBits & (1 << i)) &&
            (physicalMemoryProperties.memoryTypes[i].propertyFlags & memoryProperties)) {
            VkMemoryAllocateInfo memoryAllocateInfo = {
                VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,  // sType
                nullptr,                                 // pNext
                memoryRequirements.size,                 // allocationSize
                i                                        // memoryTypeIndex
            };
            result = vkAllocateMemory(device, &memoryAllocateInfo, nullptr, memory);
            if (result == VK_SUCCESS) {
                return true;
            }
        }
    }

    return false;
}

bool Vk_Utilities::AllocateCommandBuffers(VkCommandPool& cmdPool, uint32_t count, VkCommandBuffer* commandBuffer) {
    VkResult result = VK_SUCCESS;

    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.getVulkanDevice();

    // Allocate space in the pool for the buffer
    VkCommandBufferAllocateInfo cmdBufferAllocateInfo = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,  // sType
        nullptr,                                         // pNext
        cmdPool,                                         // commandPool
        VK_COMMAND_BUFFER_LEVEL_PRIMARY,                 // level
        count                                            // bufferCount
    };
    result = vkAllocateCommandBuffers(device, &cmdBufferAllocateInfo, commandBuffer);
    if (result != VK_SUCCESS) {
        LogError(sTag, "Could not allocate command buffer");
        return false;
    }

    return true;
}

bool Vk_Utilities::CreateVulkanSemaphore(VkSemaphore* semaphore) {
    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.getVulkanDevice();

    VkSemaphoreCreateInfo semaphoreCreateInfo = {
        VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,  // sType
        nullptr,                                  // pNext
        VkSemaphoreCreateFlags()                  // flags
    };

    VkResult result = vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, semaphore);
    if (result != VK_SUCCESS) {
        LogError(sTag, "Could not create semaphore");
        return false;
    }

    return true;
}

bool Vk_Utilities::CreateVulkanFence(VkFenceCreateFlags flags, VkFence* fence) {
    VkResult result = VK_SUCCESS;

    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.getVulkanDevice();

    VkFenceCreateInfo fenceCreateInfo = {
        VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,  // sType
        nullptr,                              // pNext
        flags                                 // flags
    };

    result = vkCreateFence(device, &fenceCreateInfo, nullptr, fence);
    if (result != VK_SUCCESS) {
        LogError(sTag, "Could not create fence");
        return false;
    }

    return true;
}

}  // namespace engine
