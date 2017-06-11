#pragma once

#include "Vk_Dependencies.hpp"

//
// Structures to encapsulate the Vulkan structs
//

namespace engine {

struct PhysicalDeviceParameters {
    vk::PhysicalDevice handle;
    vk::PhysicalDeviceProperties properties;
    vk::PhysicalDeviceFeatures features;

    operator vk::PhysicalDevice() {
        return handle;
    }
};

struct ImageParameters {
    vk::Image handle;
    vk::ImageView view;

    operator vk::Image() {
        return handle;
    }
};

struct QueueParameters {
    vk::Queue handle;
    uint32 index;
    vk::QueueFamilyProperties properties;

    QueueParameters() : index(UINT32_MAX) {}

    operator vk::Queue() {
        return handle;
    }
};

struct SwapChainParameters {
    vk::SwapchainKHR handle;
    vk::Format format;
    std::vector<ImageParameters> images;

    operator vk::SwapchainKHR() {
        return handle;
    }
};

}  // namespace engine
