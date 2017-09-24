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

    PhysicalDeviceParameters()
          : handle(VK_NULL_HANDLE),
            properties(),
            features() {}

    operator VkPhysicalDevice() {
        return handle;
    }
};

struct ImageParameters {
    VkImage handle;
    VkImageView view;

    ImageParameters() : handle(VK_NULL_HANDLE), view() {}

    operator VkImage() {
        return handle;
    }
};

struct QueueParameters {
    VkQueue handle;
    uint32 family_index;
    VkQueueFamilyProperties properties;

    QueueParameters()
          : handle(VK_NULL_HANDLE),
            family_index(UINT32_MAX),
            properties() {}

    operator VkQueue() {
        return handle;
    }
};

}  // namespace engine
