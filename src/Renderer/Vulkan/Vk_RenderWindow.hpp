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
    bool CreateVulkanPipeline();

    bool RecordCommandBuffers();

    uint32 GetVulkanSwapChainNumImages(
        const VkSurfaceCapabilitiesKHR& surface_capabilities);
    VkSurfaceFormatKHR GetVulkanSwapChainFormat(
        const std::vector<VkSurfaceFormatKHR>& surface_formats);
    VkExtent2D GetVulkanSwapChainExtent(
        const VkSurfaceCapabilitiesKHR& surface_capabilities);
    VkImageUsageFlags GetVulkanSwapChainUsageFlags(
        const VkSurfaceCapabilitiesKHR& surface_capabilities);
    VkSurfaceTransformFlagBitsKHR GetVulkanSwapChainTransform(
        const VkSurfaceCapabilitiesKHR& surface_capabilities);
    VkPresentModeKHR GetVulkanSwapChainPresentMode(
        const std::vector<VkPresentModeKHR>& present_modes);

    void ClearPipeline();
    bool OnWindowSizeChanged();

private:
    SDL_Window* m_window;

    VkSurfaceKHR m_surface;

    QueueParameters m_graphics_queue;
    QueueParameters m_present_queue;

    VkSemaphore m_image_avaliable_semaphore;
    VkSemaphore m_rendering_finished_semaphore;

    SwapChainParameters m_swapchain;
    VkPipeline m_graphics_pipeline;

    VkCommandPool m_graphics_queue_cmd_pool;
    std::vector<VkCommandBuffer> m_graphics_queue_cmd_buffers;

    VkRenderPass m_render_pass;
    std::vector<VkFramebuffer> m_framebuffers;
};

}  // namespace engine
