#pragma once

#include "Vk_Config.hpp"
#include "Vk_Dependencies.hpp"
#include "Vk_Image.hpp"
#include "Vk_Surface.hpp"

namespace engine {

class VULKAN_PLUGIN_API Vk_SwapChain : private NonCopyable {
public:
    Vk_SwapChain();
    ~Vk_SwapChain();

    bool create(Vk_Surface& surface, uint32 width, uint32 height);
    void destroy();

    VkSwapchainKHR& getHandle();
    VkFormat& getFormat();
    std::vector<Vk_Image>& getImages();

private:
    uint32 getNumImages(const VkSurfaceCapabilitiesKHR& surfaceCapabilities);
    VkSurfaceFormatKHR getFormat(const std::vector<VkSurfaceFormatKHR>& surfaceFormats);
    VkExtent2D getExtent(const VkSurfaceCapabilitiesKHR& surfaceCapabilities, uint32 width, uint32 height);
    VkImageUsageFlags getUsageFlags(const VkSurfaceCapabilitiesKHR& surfaceCapabilities);
    VkSurfaceTransformFlagBitsKHR getTransform(const VkSurfaceCapabilitiesKHR& surfaceCapabilities);
    VkPresentModeKHR getPresentMode(const std::vector<VkPresentModeKHR>& presentModes);

    VkSwapchainKHR m_handle;
    VkFormat m_format;
    std::vector<Vk_Image> m_images;
};

}  // namespace engine
