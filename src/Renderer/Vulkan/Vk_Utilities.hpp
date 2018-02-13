#pragma once

#include "Vk_Config.hpp"
#include "Vk_Dependencies.hpp"

namespace engine {

class VULKAN_PLUGIN_API Vk_Utilities {
public:
    static bool AllocateBufferMemory(
        const VkBuffer& buffer, VkDeviceMemory* memory,
        const VkMemoryPropertyFlags& memory_properties);
    static bool AllocateImageMemory(
        const VkImage& image, VkDeviceMemory* memory,
        const VkMemoryPropertyFlags& memory_properties);
    static bool AllocateMemory(VkDeviceMemory* memory,
                               const VkMemoryPropertyFlags& memory_properties,
                               const VkMemoryRequirements& memory_requirements);
};

}  // namespace engine
