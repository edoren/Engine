#include "Vk_Image.hpp"

#include <System/LogManager.hpp>
#include <System/StringFormat.hpp>
#include <System/StringView.hpp>

#include "Vk_Context.hpp"
#include "Vk_TextureManager.hpp"
#include "Vk_Utilities.hpp"

namespace engine::plugin::vulkan {

namespace {

// const StringView sTag("Vk_Image");

}  // namespace

Vk_Image::Vk_Image() : m_handle(VK_NULL_HANDLE), m_view(VK_NULL_HANDLE), m_memory(VK_NULL_HANDLE) {}

Vk_Image::Vk_Image(Vk_Image&& other) noexcept
      : m_handle(other.m_handle),
        m_view(other.m_view),
        m_memory(other.m_memory) {
    other.m_handle = VK_NULL_HANDLE;
    other.m_view = VK_NULL_HANDLE;
    other.m_memory = VK_NULL_HANDLE;
}

Vk_Image::~Vk_Image() {
    destroy();
}

bool Vk_Image::createImage(const math::uvec2& size, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage) {
    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.getVulkanDevice();

    VkResult result = VK_SUCCESS;

    VkImageCreateInfo imageCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = format,
        .extent =
            {
                .width = size.x,
                .height = size.y,
                .depth = 1,
            },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = tiling,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    result = vkCreateImage(device, &imageCreateInfo, nullptr, &m_handle);

    return result == VK_SUCCESS;
}

bool Vk_Image::createImageView(VkFormat format, VkImageAspectFlags aspectMask) {
    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.getVulkanDevice();

    VkImageViewCreateInfo imageViewCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .image = m_handle,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = format,
        .components =
            {
                .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .a = VK_COMPONENT_SWIZZLE_IDENTITY,
            },
        .subresourceRange =
            {
                .aspectMask = aspectMask,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
    };

    return vkCreateImageView(device, &imageViewCreateInfo, nullptr, &m_view) == VK_SUCCESS;
}

bool Vk_Image::allocateMemory(const VkMemoryPropertyFlags& memoryProperties) {
    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.getVulkanDevice();

    VkMemoryRequirements imageMemoryRequirements;
    vkGetImageMemoryRequirements(device, m_handle, &imageMemoryRequirements);

    if (!Vk_Utilities::AllocateMemory(&m_memory, memoryProperties, imageMemoryRequirements)) {
        return false;
    }

    if (vkBindImageMemory(device, m_handle, m_memory, 0) != VK_SUCCESS) {
        return false;
    }

    return true;
}

void Vk_Image::destroy() {
    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.getVulkanDevice();

    if (m_view != VK_NULL_HANDLE || m_handle != VK_NULL_HANDLE || m_memory != VK_NULL_HANDLE) {
        QueueParameters& graphicsQueue = context.getGraphicsQueue();
        vkQueueWaitIdle(graphicsQueue.handle);

        if (m_view != VK_NULL_HANDLE) {
            vkDestroyImageView(device, m_view, nullptr);
            m_view = VK_NULL_HANDLE;
        }

        if (m_handle != VK_NULL_HANDLE) {
            vkDestroyImage(device, m_handle, nullptr);
            m_handle = VK_NULL_HANDLE;
        }

        if (m_memory != VK_NULL_HANDLE) {
            vkFreeMemory(device, m_memory, nullptr);
            m_memory = VK_NULL_HANDLE;
        }
    }
}

VkImage& Vk_Image::getHandle() {
    return m_handle;
}

VkImageView& Vk_Image::getView() {
    return m_view;
}

}  // namespace engine::plugin::vulkan
