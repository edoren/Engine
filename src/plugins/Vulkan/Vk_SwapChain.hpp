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

    bool Create(Vk_Surface& surface, uint32 width, uint32 height);
    void Destroy();

    VkSwapchainKHR& GetHandle();
    VkFormat& GetFormat();
    std::vector<Vk_Image>& GetImages();

private:
    uint32 GetNumImages(const VkSurfaceCapabilitiesKHR& surface_capabilities);
    VkSurfaceFormatKHR GetFormat(const std::vector<VkSurfaceFormatKHR>& surface_formats);
    VkExtent2D GetExtent(const VkSurfaceCapabilitiesKHR& surface_capabilities, uint32 width, uint32 height);
    VkImageUsageFlags GetUsageFlags(const VkSurfaceCapabilitiesKHR& surface_capabilities);
    VkSurfaceTransformFlagBitsKHR GetTransform(const VkSurfaceCapabilitiesKHR& surface_capabilities);
    VkPresentModeKHR GetPresentMode(const std::vector<VkPresentModeKHR>& present_modes);

    VkSwapchainKHR m_handle;
    VkFormat m_format;
    std::vector<Vk_Image> m_images;
};

}  // namespace engine
