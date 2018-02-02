#pragma once

#include "Vk_Config.hpp"
#include "Vk_Dependencies.hpp"

namespace engine {

class VULKAN_PLUGIN_API Vk_Utilities {
public:
    static bool AllocateMemory(const VkMemoryPropertyFlags& memory_properties,
                               const VkBuffer& buffer, VkDeviceMemory* memory);
    static bool AllocateMemory(const VkMemoryPropertyFlags& memory_properties,
                               const VkImage& image, VkDeviceMemory* memory);
    static bool AllocateMemory(const VkMemoryPropertyFlags& memory_properties,
                               const VkMemoryRequirements& memory_requirements,
                               VkDeviceMemory* memory);
};

}  // namespace engine
