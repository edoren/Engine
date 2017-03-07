#pragma once

#include <Util/Prerequisites.hpp>

#include <Math/Math.hpp>
#include <Renderer/RenderWindow.hpp>
#include <System/String.hpp>

#include "Vk_Config.hpp"
#include "Vk_Dependencies.hpp"

namespace engine {

class VULKAN_PLUGIN_API Vk_RenderWindow : public RenderWindow {
public:
    Vk_RenderWindow();
    ~Vk_RenderWindow();

    ////////////////////////////////////////////////////////////////
    // Overridden
    virtual bool Create(const String& name, const math::ivec2& size);

    virtual void Destroy(void);

    virtual void Reposition(int left, int top);

    virtual void Resize(int width, int height);

    virtual void SetFullScreen(bool fullscreen, bool is_fake);

    virtual void SetVSyncEnabled(bool vsync);

    virtual void SwapBuffers();  // RenderTarget

    virtual void Clear(const Color& color);  // Render Target

    virtual bool IsVisible();
    ////////////////////////////////////////////////////////////////

private:
    bool CreateVulkanInstance();
    bool CreateVulkanSurface();
    bool CreateVulkanDevice();
    bool CreateVulkanSemaphores();
    bool CreateVulkanSwapChain();
    bool CreateVulkanQueues();
    bool CreateVulkanCommandBuffers();

    bool Draw();

    uint32 GetVulkanSwapChainNumImages(
        const vk::SurfaceCapabilitiesKHR& surface_capabilities);
    vk::SurfaceFormatKHR GetVulkanSwapChainFormat(
        const std::vector<vk::SurfaceFormatKHR>& surface_formats);
    vk::Extent2D GetVulkanSwapChainExtent(
        const vk::SurfaceCapabilitiesKHR& surface_capabilities);
    vk::ImageUsageFlags GetVulkanSwapChainUsageFlags(
        const vk::SurfaceCapabilitiesKHR& surface_capabilities);
    vk::SurfaceTransformFlagBitsKHR GetVulkanSwapChainTransform(
        const vk::SurfaceCapabilitiesKHR& surface_capabilities);
    vk::PresentModeKHR GetVulkanSwapChainPresentMode(
        const std::vector<vk::PresentModeKHR>& present_modes);

    bool CheckVulkanValidationLayerSupport() const;

    bool CheckVulkanInstanceExtensionsSupport() const;

    bool CheckPhysicalDevice(const vk::PhysicalDevice& physical_device,
                             uint32& selected_graphics_queue_family_index,
                             uint32& selected_present_queue_family_index);

private:
    SDL_Window* m_window;

    vk::Instance m_instance;
    vk::SurfaceKHR m_surface;

    vk::Device m_device;
    vk::PhysicalDevice m_physical_device;

    vk::Semaphore m_image_avaliable_semaphore;
    vk::Semaphore m_rendering_finished_semaphore;

    vk::SwapchainKHR m_swapchain;

    vk::Queue m_graphics_queue;
    vk::Queue m_present_queue;

    uint32 m_graphics_queue_family_index;
    uint32 m_present_queue_family_index;

    bool m_validation_layers_enabled;
    std::vector<const char*> m_validation_layers;

    std::vector<const char*> m_instance_extensions;
    std::vector<const char*> m_device_extensions;
};

}  // namespace engine
