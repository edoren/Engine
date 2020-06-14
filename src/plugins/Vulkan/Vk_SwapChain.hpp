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
    uint32 getNumImages(const VkSurfaceCapabilitiesKHR& surface_capabilities);
    VkSurfaceFormatKHR getFormat(const std::vector<VkSurfaceFormatKHR>& surface_formats);
    VkExtent2D getExtent(const VkSurfaceCapabilitiesKHR& surface_capabilities, uint32 width, uint32 height);
    VkImageUsageFlags getUsageFlags(const VkSurfaceCapabilitiesKHR& surface_capabilities);
    VkSurfaceTransformFlagBitsKHR getTransform(const VkSurfaceCapabilitiesKHR& surface_capabilities);
    VkPresentModeKHR getPresentMode(const std::vector<VkPresentModeKHR>& present_modes);

    VkSwapchainKHR m_handle;
    VkFormat m_format;
    std::vector<Vk_Image> m_images;
};

}  // namespace engine
