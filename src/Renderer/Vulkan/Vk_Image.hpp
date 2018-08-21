#pragma once

#include <Math/Math.hpp>

#include "Vk_Config.hpp"
#include "Vk_Dependencies.hpp"
#include "Vk_VulkanParameters.hpp"

namespace engine {

class VULKAN_PLUGIN_API Vk_Image : private NonCopyable {
public:
    Vk_Image();

    Vk_Image(Vk_Image&& other);

    virtual ~Vk_Image();

    bool CreateImage(const math::uvec2& size, VkFormat format,
                     VkImageTiling tiling, VkImageUsageFlags usage);

    bool CreateImageView(VkFormat format, VkImageAspectFlags aspectMask);

    bool AllocateMemory(const VkMemoryPropertyFlags& memory_properties);

    void Destroy();

    VkImage& GetHandle();

    VkImageView& GetView();

private:
    VkImage m_handle;
    VkImageView m_view;
    VkDeviceMemory m_memory;
};

}  // namespace engine
