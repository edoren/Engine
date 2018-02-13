#include "Vk_Utilities.hpp"
#include "Vk_Context.hpp"

namespace engine {

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

bool Vk_Utilities::AllocateImageMemory(
    const VkImage& image, VkDeviceMemory* memory,
    const VkMemoryPropertyFlags& memory_properties) {
    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.GetVulkanDevice();

    VkMemoryRequirements image_memory_requirements;
    vkGetImageMemoryRequirements(device, image, &image_memory_requirements);

    return Vk_Utilities::AllocateMemory(memory, memory_properties,
                                        image_memory_requirements);
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

}  // namespace engine
