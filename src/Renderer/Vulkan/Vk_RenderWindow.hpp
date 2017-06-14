#pragma once

#include <Util/Prerequisites.hpp>

#include <Math/Math.hpp>
#include <Renderer/RenderWindow.hpp>
#include <System/String.hpp>

#include "Vk_Config.hpp"
#include "Vk_Context.hpp"
#include "Vk_VulkanParameters.hpp"
#include "Vk_Dependencies.hpp"

namespace engine {

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
    bool CreateVulkanSurface();
    bool CreateVulkanQueues();
    bool CreateVulkanSemaphores();
    bool CreateVulkanSwapChain();
    bool CreateVulkanCommandBuffers();
    bool CreateVulkanRenderPass();
    bool CreateVulkanFrameBuffers();

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

    void CleanCommandBuffers();
    bool OnWindowSizeChanged();

private:
    SDL_Window* m_window;

    vk::SurfaceKHR m_surface;

    QueueParameters m_graphics_queue;
    QueueParameters m_present_queue;

    vk::Semaphore m_image_avaliable_semaphore;
    vk::Semaphore m_rendering_finished_semaphore;

    SwapChainParameters m_swapchain;

    vk::CommandPool m_present_queue_cmd_pool;
    std::vector<vk::CommandBuffer> m_present_queue_cmd_buffers;

    vk::RenderPass m_render_pass;
    std::vector<vk::Framebuffer> m_framebuffers;
};

}  // namespace engine
