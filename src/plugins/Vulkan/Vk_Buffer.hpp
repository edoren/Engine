#pragma once

#include <Util/Prerequisites.hpp>

#include "Vk_Config.hpp"
#include "Vk_Dependencies.hpp"

namespace engine::plugin::vulkan {

class VULKAN_PLUGIN_API Vk_Buffer {
public:
    Vk_Buffer();
    ~Vk_Buffer();

    bool create(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryProperties);

    void destroy();

    VkBuffer& getHandle();
    const VkBuffer& getHandle() const;
    VkDeviceMemory& getMemory();
    VkDeviceSize getSize() const;

private:
    VkBuffer m_handle;
    VkDeviceMemory m_memory;
    VkDeviceSize m_size;
};

}  // namespace engine::plugin::vulkan
