#pragma once

#include <Util/Prerequisites.hpp>

#include <Math/Math.hpp>
#include <Renderer/RenderWindow.hpp>
#include <System/String.hpp>

#include "Vk_Config.hpp"
#include "Vk_Context.hpp"
#include "Vk_VulkanParameters.hpp"
#include "Vk_Dependencies.hpp"
#include "Vk_Buffer.hpp"

namespace engine {

struct RenderingResourcesData {
    VkFramebuffer Framebuffer;
    VkCommandBuffer CommandBuffer;
    VkSemaphore ImageAvailableSemaphore;
    VkSemaphore FinishedRenderingSemaphore;
    VkFence Fence;

    RenderingResourcesData()
          : Framebuffer(VK_NULL_HANDLE),
            CommandBuffer(VK_NULL_HANDLE),
            ImageAvailableSemaphore(VK_NULL_HANDLE),
            FinishedRenderingSemaphore(VK_NULL_HANDLE),
            Fence(VK_NULL_HANDLE) {}
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
    bool CreateVulkanSurface();
    bool CreateVulkanQueues();
    bool CreateVulkanSemaphores();
    bool CreateVulkanFences();
    bool CreateVulkanSwapChain();
    bool CreateVulkanCommandBuffers();
    bool CreateVulkanRenderPass();
    bool CreateVulkanPipeline();
    bool CreateVulkanVertexBuffer();

    bool CreateVulkanFrameBuffer(VkFramebuffer& framebuffer,
                                 VkImageView& image_view);
    bool PrepareFrame(VkCommandBuffer command_buffer,
                      ImageParameters& image_parameters,
                      VkFramebuffer& framebuffer);

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

    bool AllocateVulkanBufferMemory(VkBuffer buffer, VkDeviceMemory* memory);

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

    VkRenderPass m_render_pass;

    Vk_Buffer m_vertex_buffer;
    std::vector<RenderingResourcesData> m_render_resources;
};

}  // namespace engine
