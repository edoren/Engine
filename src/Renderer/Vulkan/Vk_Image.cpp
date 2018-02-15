#include "Vk_Image.hpp"

#include <System/LogManager.hpp>
#include <System/StringFormat.hpp>

#include "Vk_Context.hpp"
#include "Vk_TextureManager.hpp"
#include "Vk_Utilities.hpp"

namespace engine {

namespace {

const String sTag("Vk_Image");

}  // namespace

Vk_Image::Vk_Image() {}

Vk_Image::~Vk_Image() {
    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.GetVulkanDevice();
    QueueParameters& graphics_queue = context.GetGraphicsQueue();

    vkQueueWaitIdle(graphics_queue.handle);

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

bool Vk_Image::CreateImage(const math::uvec2& size, VkFormat format,
                           VkImageTiling tiling, VkImageUsageFlags usage) {
    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.GetVulkanDevice();

    VkResult result = VK_SUCCESS;

    VkImageCreateInfo image_create_info = {
        VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,  // sType;
        nullptr,                              // pNext
        0,                                    // flags
        VK_IMAGE_TYPE_2D,                     // imageType
        format,                               // format
        VkExtent3D{
            // extent
            size.x,  // width
            size.y,  // height
            1        // depth
        },
        1,                          // mipLevels
        1,                          // arrayLayers
        VK_SAMPLE_COUNT_1_BIT,      // samples
        tiling,                     // tiling
        usage,                      // usage
        VK_SHARING_MODE_EXCLUSIVE,  // sharingMode
        0,                          // queueFamilyIndexCount
        nullptr,                    // pQueueFamilyIndices
        VK_IMAGE_LAYOUT_UNDEFINED   // initialLayout
    };

    result = vkCreateImage(device, &image_create_info, nullptr, &m_handle);

    return result == VK_SUCCESS;
}

bool Vk_Image::CreateImageView(VkFormat format, VkImageAspectFlags aspectMask) {
    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.GetVulkanDevice();

    VkImageViewCreateInfo image_view_create_info = {
        VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,  // sType
        nullptr,                                   // pNext
        0,                                         // flags
        m_handle,                                  // image
        VK_IMAGE_VIEW_TYPE_2D,                     // viewType
        format,                                    // format
        {
            // components
            VK_COMPONENT_SWIZZLE_IDENTITY,  // r
            VK_COMPONENT_SWIZZLE_IDENTITY,  // g
            VK_COMPONENT_SWIZZLE_IDENTITY,  // b
            VK_COMPONENT_SWIZZLE_IDENTITY   // a
        },
        {
            // subresourceRange
            aspectMask,  // aspectMask
            0,           // baseMipLevel
            1,           // levelCount
            0,           // baseArrayLayer
            1            // layerCount
        },
    };

    return vkCreateImageView(device, &image_view_create_info, nullptr,
                             &m_view) == VK_SUCCESS;
}

bool Vk_Image::AllocateMemory(const VkMemoryPropertyFlags& memory_properties) {
    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.GetVulkanDevice();

    VkMemoryRequirements image_memory_requirements;
    vkGetImageMemoryRequirements(device, m_handle, &image_memory_requirements);

    if (!Vk_Utilities::AllocateMemory(&m_memory, memory_properties,
                                      image_memory_requirements)) {
        return false;
    }

    if (vkBindImageMemory(device, m_handle, m_memory, 0) != VK_SUCCESS) {
        return false;
    }

    return true;
}

VkImage& Vk_Image::GetHandle() {
    return m_handle;
}

VkImageView& Vk_Image::GetView() {
    return m_view;
}

}  // namespace engine
