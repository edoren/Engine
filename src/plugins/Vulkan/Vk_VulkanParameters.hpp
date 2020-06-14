#pragma once

#include "Vk_Dependencies.hpp"

//
// Structures to encapsulate the Vulkan structs
//

namespace engine {

struct PhysicalDeviceParameters {
    VkPhysicalDevice handle;
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;

    PhysicalDeviceParameters() : handle(VK_NULL_HANDLE), properties(), features() {}

    operator VkPhysicalDevice() const {
        return handle;
    }

    VkPhysicalDevice& getHandle() {
        return handle;
    }

    VkFormatProperties getFormatProperties(VkFormat format) const {
        VkFormatProperties format_properties;
        vkGetPhysicalDeviceFormatProperties(handle, format, &format_properties);
        return format_properties;
    }
};

struct QueueParameters {
    VkQueue handle;
    uint32 family_index;
    VkQueueFamilyProperties properties;

    QueueParameters() : handle(VK_NULL_HANDLE), family_index(UINT32_MAX), properties() {}

    operator VkQueue() const {
        return handle;
    }

    VkQueue& getHandle() {
        return handle;
    }
};

struct DescriptorSetParameters {
    VkDescriptorSet handle;
    VkDescriptorPool pool;
    VkDescriptorSetLayout layout;

    DescriptorSetParameters() : handle(VK_NULL_HANDLE), pool(VK_NULL_HANDLE), layout(VK_NULL_HANDLE) {}

    operator VkDescriptorSet() const {
        return handle;
    }

    VkDescriptorSet& getHandle() {
        return handle;
    }
};

}  // namespace engine
