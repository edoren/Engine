#pragma once

#include <Math/Math.hpp>
#include <Util/NonCopyable.hpp>

#include "Vk_Config.hpp"
#include "Vk_Dependencies.hpp"
#include "Vk_VulkanParameters.hpp"

namespace engine::plugin::vulkan {

class VULKAN_PLUGIN_API Vk_Image : private NonCopyable {
public:
    Vk_Image();

    Vk_Image(Vk_Image&& other) noexcept;

    ~Vk_Image();

    bool createImage(const math::uvec2& size, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage);

    bool createImageView(VkFormat format, VkImageAspectFlags aspectMask);

    bool allocateMemory(const VkMemoryPropertyFlags& memoryProperties);

    void destroy();

    VkImage& getHandle();

    VkImageView& getView();

private:
    VkImage m_handle;
    VkImageView m_view;
    VkDeviceMemory m_memory;
};

}  // namespace engine::plugin::vulkan
