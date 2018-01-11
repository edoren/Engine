#pragma once

#include <Util/Prerequisites.hpp>

#include "Vk_Config.hpp"
#include "Vk_Dependencies.hpp"

namespace engine {

class VULKAN_PLUGIN_API Vk_Buffer {
public:
    Vk_Buffer();
    ~Vk_Buffer();

    bool Create(VkDeviceSize size, VkBufferUsageFlags usage,
                VkMemoryPropertyFlags memory_properties);

    void Destroy();

    VkBuffer& GetHandle();
    VkDeviceMemory& GetMemory();
    VkDeviceSize GetSize() const;

private:
    bool AllocateMemory(VkMemoryPropertyFlags memory_properties);

    VkBuffer m_handle;
    VkDeviceMemory m_memory;
    VkDeviceSize m_size;
};

}  // namespace engine