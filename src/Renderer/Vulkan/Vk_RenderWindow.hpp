#pragma once

#include <Util/Prerequisites.hpp>

#include <Math/Math.hpp>
#include <Renderer/RenderWindow.hpp>
#include <System/String.hpp>

#include "Vk_Config.hpp"
#include "Vk_Dependencies.hpp"

namespace engine {

struct ImageParameters {
    vk::Image handle;
    vk::ImageView view;
};

struct QueueParameters {
    vk::Queue handle;
    uint32 index;
};

struct SwapChainParameters {
    vk::SwapchainKHR handle;
    vk::Format format;
    std::vector<ImageParameters> images;
};

class VULKAN_PLUGIN_API Vk_RenderWindow : public RenderWindow {
public:
    Vk_RenderWindow();

    ~Vk_RenderWindow();

    virtual bool Create(const String& name, const math::ivec2& size);

    virtual void Destroy(void);

    virtual void Reposition(int left, int top);

    virtual void Resize(int width, int height);

    virtual void SetFullScreen(bool fullscreen, bool is_fake);

    virtual void SetVSyncEnabled(bool vsync);

    virtual void SwapBuffers();  // RenderTarget

    virtual void Clear(const Color& color);  // Render Target

    virtual bool IsVisible();

private:
    bool CreateVulkanInstance();
    bool CreateVulkanSurface();
    bool CreateVulkanDevice();
    bool CreateVulkanSemaphores();
    bool CreateVulkanSwapChain();
    bool CreateVulkanQueues();
    bool CreateVulkanCommandBuffers();
    bool CreateVulkanRenderPass();
    bool CreateVulkanFrameBuffer();

    bool RecordCommandBuffers();

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

    void CleanCommandBuffers();
    bool OnWindowSizeChanged();

private:
    SDL_Window* m_window;

    vk::Instance m_instance;
    vk::SurfaceKHR m_surface;

    vk::Device m_device;
    vk::PhysicalDevice m_physical_device;

    vk::Semaphore m_image_avaliable_semaphore;
    vk::Semaphore m_rendering_finished_semaphore;

    SwapChainParameters m_swapchain;

    QueueParameters m_graphics_queue;
    QueueParameters m_present_queue;

    vk::CommandPool m_present_queue_cmd_pool;
    std::vector<vk::CommandBuffer> m_present_queue_cmd_buffers;

    vk::RenderPass m_render_pass;
    std::vector<vk::Framebuffer> m_framebuffers;

    bool m_validation_layers_enabled;
    std::vector<const char*> m_validation_layers;

    std::vector<const char*> m_instance_extensions;
    std::vector<const char*> m_device_extensions;
};

}  // namespace engine
