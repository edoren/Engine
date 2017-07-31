#pragma once

#include <Util/Prerequisites.hpp>

#include <Math/Math.hpp>
#include <Renderer/RenderWindow.hpp>
#include <System/String.hpp>

#include "Vk_Config.hpp"
#include "Vk_Context.hpp"
#include "Vk_VulkanParameters.hpp"
#include "Vk_Dependencies.hpp"

namespace engine {

class VULKAN_PLUGIN_API Vk_Buffer {
public:
    Vk_Buffer() : handle(VK_NULL_HANDLE), memory(VK_NULL_HANDLE), size(0) {}

    operator VkBuffer() {
        return handle;
    }

public:
    VkBuffer handle;
    VkDeviceMemory memory;
    uint32_t size;
};

}  // namespace engine
