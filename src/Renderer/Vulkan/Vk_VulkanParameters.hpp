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
    uint32 index;
    VkQueueFamilyProperties properties;

    QueueParameters()
          : handle(VK_NULL_HANDLE),
            index(UINT32_MAX),
            properties() {}

    operator VkQueue() {
        return handle;
    }
};

struct SwapChainParameters {
    VkSwapchainKHR handle;
    VkFormat format;
    std::vector<ImageParameters> images;

    SwapChainParameters() : handle(VK_NULL_HANDLE), format(), images() {}

    operator VkSwapchainKHR() {
        return handle;
    }
};

}  // namespace engine
