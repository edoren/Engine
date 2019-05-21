#include <Math/Math.hpp>
#include <System/LogManager.hpp>
#include <System/StringFormat.hpp>

#include "Vk_Context.hpp"
#include "Vk_SwapChain.hpp"

namespace engine {

namespace {

const String sTag("Vk_SwapChain");

}  // namespace

Vk_SwapChain::Vk_SwapChain()
      : m_handle(VK_NULL_HANDLE), m_format(), m_images() {}

Vk_SwapChain::~Vk_SwapChain() {
    if (m_handle) {
        Destroy();
    }
}

bool Vk_SwapChain::Create(Vk_Surface& surface, uint32 width, uint32 height) {
    VkResult result = VK_SUCCESS;

    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.GetVulkanDevice();
    VkPhysicalDevice physical_device = context.GetPhysicalDevice();

    // Wait all the Device Queues to finish
    if (device) {
        vkDeviceWaitIdle(device);
    }

    // Destroy the old ImageViews
    for (auto& image : m_images) {
        // Image handles are managed by the swapchain
        // and must be destroyed my it
        image.GetHandle() = VK_NULL_HANDLE;
    }
    m_images.clear();

    // Get the Surface capabilities
    VkSurfaceCapabilitiesKHR surface_capabilities;

    result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        physical_device, surface.GetHandle(), &surface_capabilities);
    if (result != VK_SUCCESS) {
        LogError(sTag, "Could not check presentation surface capabilities");
        return false;
    }

    // Query all the supported Surface formats
    uint32 formats_count = 0;
    std::vector<VkSurfaceFormatKHR> surface_formats;
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(
        physical_device, surface.GetHandle(), &formats_count, nullptr);
    if (formats_count > 0 && result == VK_SUCCESS) {
        surface_formats.resize(formats_count);
        result = vkGetPhysicalDeviceSurfaceFormatsKHR(
            physical_device, surface.GetHandle(), &formats_count,
            surface_formats.data());
    }

    // Check that the surface formats where queried successfully
    if (formats_count == 0 || result != VK_SUCCESS) {
        LogError(sTag,
                 "Error occurred during presentation surface formats "
                 "enumeration");
        return false;
    }

    // Query all the supported Surface present modes
    uint32 present_modes_count = 0;
    std::vector<VkPresentModeKHR> present_modes;

    result = vkGetPhysicalDeviceSurfacePresentModesKHR(
        physical_device, surface.GetHandle(), &present_modes_count, nullptr);
    if (present_modes_count > 0 && result == VK_SUCCESS) {
        present_modes.resize(present_modes_count);
        result = vkGetPhysicalDeviceSurfacePresentModesKHR(
            physical_device, surface.GetHandle(), &present_modes_count,
            present_modes.data());
    }

    // Check that the surface present modes where queried successfully
    if (present_modes_count == 0 || result != VK_SUCCESS) {
        LogError(sTag,
                 "Error occurred during presentation surface formats "
                 "enumeration");
        return false;
    }

    // Retreive all the Swapchain related information
    uint32 desired_number_of_images = GetNumImages(surface_capabilities);
    VkSurfaceFormatKHR desired_format = GetFormat(surface_formats);
    VkExtent2D desired_extent = GetExtent(surface_capabilities, width, height);
    VkImageUsageFlags desired_usage = GetUsageFlags(surface_capabilities);
    VkSurfaceTransformFlagBitsKHR desired_transform =
        GetTransform(surface_capabilities);
    VkPresentModeKHR desired_present_mode = GetPresentMode(present_modes);
    VkSwapchainKHR old_swap_chain = m_handle;

    if (static_cast<int>(desired_usage) == -1) {
        return false;
    }

    VkCompositeAlphaFlagBitsKHR composite_alpha =
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    if (!(surface_capabilities.supportedCompositeAlpha & composite_alpha)) {
        composite_alpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
    }

    VkSwapchainCreateInfoKHR swap_chain_create_info = {
        VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,  // sType
        nullptr,                                      // pNext
        VkSwapchainCreateFlagsKHR(),                  // flags
        surface.GetHandle(),                          // surface
        desired_number_of_images,                     // minImageCount
        desired_format.format,                        // imageFormat
        desired_format.colorSpace,                    // imageColorSpace
        desired_extent,                               // imageExtent
        1,                                            // imageArrayLayers
        desired_usage,                                // imageUsage
        VK_SHARING_MODE_EXCLUSIVE,                    // imageSharingMode
        0,                                            // queueFamilyIndexCount
        nullptr,                                      // pQueueFamilyIndices
        desired_transform,                            // preTransform
        composite_alpha,                              // compositeAlpha
        desired_present_mode,                         // presentMode
        true,                                         // clipped
        old_swap_chain                                // oldSwapchain
    };

    result = vkCreateSwapchainKHR(device, &swap_chain_create_info, nullptr,
                                  &m_handle);
    if (result != VK_SUCCESS) {
        LogFatal(sTag, "Could not create swap chain");
        return false;
    }

    LogInfo(sTag, "SwapChain created with presentation mode: {}"_format(
                      desired_present_mode));

    if (old_swap_chain) {
        vkDestroySwapchainKHR(device, old_swap_chain, nullptr);
    }

    // Store all the necesary SwapChain parameters

    // Get the SwapChain format
    m_format = desired_format.format;

    // Get the SwapChain images
    uint32 image_count = 0;
    std::vector<VkImage> swapchain_images;
    result = vkGetSwapchainImagesKHR(device, m_handle, &image_count, nullptr);

    if (image_count > 0 && result == VK_SUCCESS) {
        swapchain_images.resize(image_count);
        result = vkGetSwapchainImagesKHR(device, m_handle, &image_count,
                                         swapchain_images.data());
    }
    // Check that all the images could be queried
    if (image_count == 0 || result != VK_SUCCESS) {
        LogError(sTag, "Could not get the number of swap chain images");
        return false;
    }
    // Store all the Image handles
    m_images.resize(image_count);
    for (size_t i = 0; i < swapchain_images.size(); i++) {
        m_images[i].GetHandle() = swapchain_images[i];
    }
    // Create all the ImageViews
    for (size_t i = 0; i < m_images.size(); i++) {
        VkImageViewCreateInfo image_view_create_info = {
            VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,  // sType
            nullptr,                                   // pNext
            VkImageViewCreateFlags(),                  // flags
            m_images[i].GetHandle(),                   // image
            VK_IMAGE_VIEW_TYPE_2D,                     // viewType
            m_format,                                  // format
            VkComponentMapping{
                // components
                VK_COMPONENT_SWIZZLE_IDENTITY,  // r
                VK_COMPONENT_SWIZZLE_IDENTITY,  // g
                VK_COMPONENT_SWIZZLE_IDENTITY,  // b
                VK_COMPONENT_SWIZZLE_IDENTITY   // a
            },
            VkImageSubresourceRange{
                VK_IMAGE_ASPECT_COLOR_BIT,  // aspectMask
                0,                          // baseMipLevel
                1,                          // levelCount
                0,                          // baseArrayLayer
                1                           // layerCount
            },
        };

        result = vkCreateImageView(context.GetVulkanDevice(),
                                   &image_view_create_info, nullptr,
                                   &m_images[i].GetView());
        if (result != VK_SUCCESS) {
            LogError(sTag, "Could not create image view for framebuffer");
            return false;
        }
    }

    return true;
}

void Vk_SwapChain::Destroy() {
    if (m_handle) {
        Vk_Context& context = Vk_Context::GetInstance();
        VkDevice& device = context.GetVulkanDevice();
        vkDeviceWaitIdle(device);
        vkDestroySwapchainKHR(device, m_handle, nullptr);
        m_handle = VK_NULL_HANDLE;
        for (auto& image : m_images) {
            // Image handles are managed by the swapchain
            // and must be destroyed my it
            image.GetHandle() = VK_NULL_HANDLE;
        }
        m_images.clear();
    }
}

VkSwapchainKHR& Vk_SwapChain::GetHandle() {
    return m_handle;
}

VkFormat& Vk_SwapChain::GetFormat() {
    return m_format;
}

std::vector<Vk_Image>& Vk_SwapChain::GetImages() {
    return m_images;
}

uint32 Vk_SwapChain::GetNumImages(
    const VkSurfaceCapabilitiesKHR& surface_capabilities) {
    // Set of images defined in a swap chain may not always be available for
    // application to render to:
    // One may be displayed and one may wait in a queue to be presented
    // If application wants to use more images at the same time it must ask
    // for more images
    uint32 image_count = surface_capabilities.minImageCount + 1;
    if (surface_capabilities.maxImageCount > 0 &&
        image_count > surface_capabilities.maxImageCount) {
        image_count = surface_capabilities.maxImageCount;
    }
    return image_count;
}

VkSurfaceFormatKHR Vk_SwapChain::GetFormat(
    const std::vector<VkSurfaceFormatKHR>& surface_formats) {
    // If the list contains only one entry with undefined format
    // it means that there are no preferred surface formats and any can be
    // chosen
    if (surface_formats.size() == 1 &&
        surface_formats[0].format == VK_FORMAT_UNDEFINED) {
        return {VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
    }

    // Check if list contains most widely used R8 G8 B8 A8 format
    // with nonlinear color space
    for (const VkSurfaceFormatKHR& surface_format : surface_formats) {
        if (surface_format.format == VK_FORMAT_R8G8B8A8_UNORM) {
            return surface_format;
        }
    }

    // Return the first format from the list
    return surface_formats[0];
}

VkExtent2D Vk_SwapChain::GetExtent(
    const VkSurfaceCapabilitiesKHR& surface_capabilities, uint32 width,
    uint32 height) {
    // Special value of surface extent is width == height == 0xFFFFFFFF
    // If this is so we define the size by ourselves but it must fit within
    // defined confines
    if (surface_capabilities.currentExtent.width == 0xFFFFFFFF) {
        VkExtent2D swap_chain_extent = {width, height};
        swap_chain_extent.width = math::Clamp(
            swap_chain_extent.width, surface_capabilities.minImageExtent.width,
            surface_capabilities.maxImageExtent.width);
        swap_chain_extent.height =
            math::Clamp(swap_chain_extent.height,
                        surface_capabilities.minImageExtent.height,
                        surface_capabilities.maxImageExtent.height);
        return swap_chain_extent;
    }

    // Most of the cases we define size of the swap_chain images equal to
    // current window's size
    return surface_capabilities.currentExtent;
}

VkImageUsageFlags Vk_SwapChain::GetUsageFlags(
    const VkSurfaceCapabilitiesKHR& surface_capabilities) {
    // Color attachment flag must always be supported
    // We can define other usage flags but we always need to check if they
    // are supported
    if (surface_capabilities.supportedUsageFlags &
        VK_IMAGE_USAGE_TRANSFER_DST_BIT) {
        return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
               VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }
    return VkImageUsageFlags(static_cast<VkImageUsageFlagBits>(-1));
}

VkSurfaceTransformFlagBitsKHR Vk_SwapChain::GetTransform(
    const VkSurfaceCapabilitiesKHR& surface_capabilities) {
    // Sometimes images must be transformed before they are presented (i.e.
    // due to device's orienation being other than default orientation)
    // If the specified transform is other than current transform,
    // presentation engine will transform image during presentation operation;
    // this operation
    // may hit performance on some platforms
    // Here we don't want any transformations to occur so if the identity
    // transform is supported use it otherwise just use the same transform
    // as current transform
    if (surface_capabilities.supportedTransforms &
        VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
        return VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    } else {
        return surface_capabilities.currentTransform;
    }
}

VkPresentModeKHR Vk_SwapChain::GetPresentMode(
    const std::vector<VkPresentModeKHR>& present_modes) {
    // MAILBOX is the lowest latency V-Sync enabled mode (something like
    // triple-buffering) so use it if available
    for (const VkPresentModeKHR& present_mode : present_modes) {
        if (present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return present_mode;
        }
    }
    // FIFO is the only present mode that is required to be supported
    // by the VulkanSDK.
    return VK_PRESENT_MODE_FIFO_KHR;
}

}  // namespace engine
