#include <Math/Math.hpp>
#include <System/LogManager.hpp>
#include <System/StringFormat.hpp>
#include <System/StringView.hpp>
#include <Util/Container/Vector.hpp>

#include "Vk_Context.hpp"
#include "Vk_SwapChain.hpp"

namespace engine::plugin::vulkan {

namespace {

const StringView sTag("Vk_SwapChain");

const char* GetPresentationModeString(VkPresentModeKHR mode) {
    switch (mode) {
        case VK_PRESENT_MODE_IMMEDIATE_KHR:
            return "IMMEDIATE";
        case VK_PRESENT_MODE_MAILBOX_KHR:
            return "MAILBOX";
        case VK_PRESENT_MODE_FIFO_KHR:
            return "FIFO";
        case VK_PRESENT_MODE_FIFO_RELAXED_KHR:
            return "FIFO_RELAXED";
        case VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR:
            return "SHARED_DEMAND_REFRESH";
        case VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR:
            return "SHARED_CONTINUOUS_REFRESH";
        default:
            return "UNKNOWN";
    }
}

}  // namespace

Vk_SwapChain::Vk_SwapChain() : m_handle(VK_NULL_HANDLE), m_format() {}

Vk_SwapChain::~Vk_SwapChain() {
    if (m_handle) {
        destroy();
    }
}

bool Vk_SwapChain::create(Vk_Surface& surface, uint32 width, uint32 height) {
    VkResult result = VK_SUCCESS;

    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.getVulkanDevice();
    VkPhysicalDevice physicalDevice = context.getPhysicalDevice();

    // Wait all the Device Queues to finish
    if (device) {
        vkDeviceWaitIdle(device);
    }

    // Get the Surface capabilities
    VkSurfaceCapabilitiesKHR surfaceCapabilities = {};

    result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface.getHandle(), &surfaceCapabilities);
    if (result != VK_SUCCESS) {
        LogError(sTag, "Could not check presentation surface capabilities");
        return false;
    }

    // Destroy the old ImageViews
    for (auto& image : m_images) {
        // Image handles are managed by the swapchain
        // and must be destroyed my it
        image.getHandle() = VK_NULL_HANDLE;
    }
    m_images.clear();

    // Query all the supported Surface formats
    uint32 formatsCount = 0;
    Vector<VkSurfaceFormatKHR> surfaceFormats;
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface.getHandle(), &formatsCount, nullptr);
    if (formatsCount > 0 && result == VK_SUCCESS) {
        surfaceFormats.resize(formatsCount);
        result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface.getHandle(), &formatsCount,
                                                      surfaceFormats.data());
    }

    // Check that the surface formats where queried successfully
    if (formatsCount == 0 || result != VK_SUCCESS) {
        LogError(sTag, "Error occurred during presentation surface formats enumeration");
        return false;
    }

    // Query all the supported Surface present modes
    uint32 presentModesCount = 0;
    Vector<VkPresentModeKHR> presentModes;

    result =
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface.getHandle(), &presentModesCount, nullptr);
    if (presentModesCount > 0 && result == VK_SUCCESS) {
        presentModes.resize(presentModesCount);
        result = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface.getHandle(), &presentModesCount,
                                                           presentModes.data());
    }

    // Check that the surface present modes where queried successfully
    if (presentModesCount == 0 || result != VK_SUCCESS) {
        LogError(sTag, "Error occurred during presentation surface formats enumeration");
        return false;
    }

    // Retreive all the Swapchain related information
    uint32 desiredNumberOfImages = getNumImages(surfaceCapabilities);
    VkSurfaceFormatKHR desiredFormat = getFormat(surfaceFormats);
    VkExtent2D desiredExtent = getExtent(surfaceCapabilities, width, height);
    VkImageUsageFlags desiredUsage = getUsageFlags(surfaceCapabilities);
    VkSurfaceTransformFlagBitsKHR desiredTransform = getTransform(surfaceCapabilities);
    VkPresentModeKHR desiredPresentMode = getPresentMode(presentModes);
    VkSwapchainKHR oldSwapChain = m_handle;

    if (static_cast<int>(desiredUsage) == -1) {
        return false;
    }

    VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    if (!(surfaceCapabilities.supportedCompositeAlpha & compositeAlpha)) {
        compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
    }

    VkSwapchainCreateInfoKHR swapChainCreateInfo = {
        VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,  // sType
        nullptr,                                      // pNext
        VkSwapchainCreateFlagsKHR(),                  // flags
        surface.getHandle(),                          // surface
        desiredNumberOfImages,                        // minImageCount
        desiredFormat.format,                         // imageFormat
        desiredFormat.colorSpace,                     // imageColorSpace
        desiredExtent,                                // imageExtent
        1,                                            // imageArrayLayers
        desiredUsage,                                 // imageUsage
        VK_SHARING_MODE_EXCLUSIVE,                    // imageSharingMode
        0,                                            // queueFamilyIndexCount
        nullptr,                                      // pQueueFamilyIndices
        desiredTransform,                             // preTransform
        compositeAlpha,                               // compositeAlpha
        desiredPresentMode,                           // presentMode
        true,                                         // clipped
        oldSwapChain                                  // oldSwapchain
    };

    result = vkCreateSwapchainKHR(device, &swapChainCreateInfo, nullptr, &m_handle);
    if (result != VK_SUCCESS) {
        LogFatal(sTag, "Could not create swap chain");
        return false;
    }

    const char* presentModeStr = GetPresentationModeString(desiredPresentMode);
    LogInfo(sTag, "SwapChain created with presentation mode {} and dimensions [{}, {}]", presentModeStr,
            desiredExtent.width, desiredExtent.height);

    if (oldSwapChain) {
        vkDestroySwapchainKHR(device, oldSwapChain, nullptr);
    }

    // Store all the necesary SwapChain parameters

    // Get the SwapChain format
    m_format = desiredFormat.format;

    // Get the SwapChain images
    uint32 imageCount = 0;
    Vector<VkImage> swapchainImages;
    result = vkGetSwapchainImagesKHR(device, m_handle, &imageCount, nullptr);

    if (imageCount > 0 && result == VK_SUCCESS) {
        swapchainImages.resize(imageCount);
        result = vkGetSwapchainImagesKHR(device, m_handle, &imageCount, swapchainImages.data());
    }
    // Check that all the images could be queried
    if (imageCount == 0 || result != VK_SUCCESS) {
        LogError(sTag, "Could not get the number of swap chain images");
        return false;
    }
    // Store all the Image handles
    m_images.resize(imageCount);
    for (size_t i = 0; i < swapchainImages.size(); i++) {
        m_images[i].getHandle() = swapchainImages[i];
    }
    // Create all the ImageViews
    for (auto& image : m_images) {
        VkImageViewCreateInfo imageViewCreateInfo = {
            VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,  // sType
            nullptr,                                   // pNext
            VkImageViewCreateFlags(),                  // flags
            image.getHandle(),                         // image
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

        result = vkCreateImageView(context.getVulkanDevice(), &imageViewCreateInfo, nullptr, &image.getView());
        if (result != VK_SUCCESS) {
            LogError(sTag, "Could not create image view for framebuffer");
            return false;
        }
    }

    return true;
}

void Vk_SwapChain::destroy() {
    if (m_handle) {
        Vk_Context& context = Vk_Context::GetInstance();
        VkDevice& device = context.getVulkanDevice();
        vkDeviceWaitIdle(device);
        vkDestroySwapchainKHR(device, m_handle, nullptr);
        m_handle = VK_NULL_HANDLE;
        for (auto& image : m_images) {
            // Image handles are managed by the swapchain
            // and must be destroyed my it
            image.getHandle() = VK_NULL_HANDLE;
        }
        m_images.clear();
    }
}

VkSwapchainKHR& Vk_SwapChain::getHandle() {
    return m_handle;
}

VkFormat& Vk_SwapChain::getFormat() {
    return m_format;
}

Vector<Vk_Image>& Vk_SwapChain::getImages() {
    return m_images;
}

uint32 Vk_SwapChain::getNumImages(const VkSurfaceCapabilitiesKHR& surfaceCapabilities) {
    // Set of images defined in a swap chain may not always be available for
    // application to render to:
    // One may be displayed and one may wait in a queue to be presented
    // If application wants to use more images at the same time it must ask
    // for more images
    uint32 imageCount = surfaceCapabilities.minImageCount + 1;
    if (surfaceCapabilities.maxImageCount > 0 && imageCount > surfaceCapabilities.maxImageCount) {
        imageCount = surfaceCapabilities.maxImageCount;
    }
    return imageCount;
}

VkSurfaceFormatKHR Vk_SwapChain::getFormat(const Vector<VkSurfaceFormatKHR>& surfaceFormats) {
    // If the list contains only one entry with undefined format
    // it means that there are no preferred surface formats and any can be
    // chosen
    if (surfaceFormats.size() == 1 && surfaceFormats[0].format == VK_FORMAT_UNDEFINED) {
        return {VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
    }

    // Check if list contains most widely used R8 G8 B8 A8 format
    // with nonlinear color space
    for (const VkSurfaceFormatKHR& surfaceFormat : surfaceFormats) {
        if (surfaceFormat.format == VK_FORMAT_R8G8B8A8_UNORM) {
            return surfaceFormat;
        }
    }

    // Return the first format from the list
    return surfaceFormats[0];
}

VkExtent2D Vk_SwapChain::getExtent(const VkSurfaceCapabilitiesKHR& surfaceCapabilities, uint32 width, uint32 height) {
    // Special value of surface extent is width == height == 0xFFFFFFFF
    // If this is so we define the size by ourselves but it must fit within
    // defined confines
    if (surfaceCapabilities.currentExtent.width == 0xFFFFFFFF) {
        VkExtent2D swapChainExtent = {width, height};
        swapChainExtent.width = math::Clamp(swapChainExtent.width, surfaceCapabilities.minImageExtent.width,
                                            surfaceCapabilities.maxImageExtent.width);
        swapChainExtent.height = math::Clamp(swapChainExtent.height, surfaceCapabilities.minImageExtent.height,
                                             surfaceCapabilities.maxImageExtent.height);
        return swapChainExtent;
    }

    // Most of the cases we define size of the swap_chain images equal to
    // current window's size
    return surfaceCapabilities.currentExtent;
}

VkImageUsageFlags Vk_SwapChain::getUsageFlags(const VkSurfaceCapabilitiesKHR& surfaceCapabilities) {
    // Color attachment flag must always be supported
    // We can define other usage flags but we always need to check if they
    // are supported
    if (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) {
        return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }
    return VkImageUsageFlags(static_cast<VkImageUsageFlagBits>(-1));
}

VkSurfaceTransformFlagBitsKHR Vk_SwapChain::getTransform(const VkSurfaceCapabilitiesKHR& surfaceCapabilities) {
    // Sometimes images must be transformed before they are presented (i.e.
    // due to device's orienation being other than default orientation)
    // If the specified transform is other than current transform,
    // presentation engine will transform image during presentation operation;
    // this operation
    // may hit performance on some platforms
    // Here we don't want any transformations to occur so if the identity
    // transform is supported use it otherwise just use the same transform
    // as current transform
    if (surfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
        return VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    }
    return surfaceCapabilities.currentTransform;
}

VkPresentModeKHR Vk_SwapChain::getPresentMode(const Vector<VkPresentModeKHR>& presentModes) {
    ENGINE_UNUSED(presentModes);
    // MAILBOX is the lowest latency V-Sync enabled mode (something like
    // triple-buffering) so use it if available
    // for (const VkPresentModeKHR& present_mode : present_modes) {
    //     if (present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
    //         return present_mode;
    //     }
    // }
    // FIFO is the only present mode that is required to be supported
    // by the VulkanSDK.
    return VK_PRESENT_MODE_FIFO_KHR;
}

}  // namespace engine::plugin::vulkan
