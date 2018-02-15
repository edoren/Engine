#pragma once

#include <Math/Math.hpp>

#include "Vk_Config.hpp"
#include "Vk_Dependencies.hpp"
#include "Vk_VulkanParameters.hpp"

namespace engine {

class Vk_Image {
public:
    Vk_Image();

    virtual ~Vk_Image();

    bool CreateImage(const math::uvec2& size, VkFormat format,
                     VkImageTiling tiling, VkImageUsageFlags usage);

    bool CreateImageView(VkFormat format, VkImageAspectFlags aspectMask);

    bool AllocateMemory(const VkMemoryPropertyFlags& memory_properties);

    VkImage& GetHandle();

    VkImageView& GetView();

private:
    VkImage m_handle;
    VkImageView m_view;
    VkDeviceMemory m_memory;
};

}  // namespace engine
