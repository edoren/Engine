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
    VkFramebuffer framebuffer;
    VkCommandBuffer command_buffer;
    VkSemaphore image_available_semaphore;
    VkSemaphore finished_rendering_semaphore;
    VkFence fence;

    RenderingResourcesData()
          : framebuffer(VK_NULL_HANDLE),
            command_buffer(VK_NULL_HANDLE),
            image_available_semaphore(VK_NULL_HANDLE),
            finished_rendering_semaphore(VK_NULL_HANDLE),
            fence(VK_NULL_HANDLE) {}
};

class VULKAN_PLUGIN_API Vk_RenderWindow : public RenderWindow {
public:
    Vk_RenderWindow();

    ~Vk_RenderWindow();

    virtual bool Create(const String& name, const math::ivec2& size) override;

    virtual void Destroy(void) override;

    virtual void Reposition(int left, int top) override;

    virtual void Resize(int width, int height) override;

    virtual void SetFullScreen(bool fullscreen, bool is_fake) override;

    virtual void SetVSyncEnabled(bool vsync) override;

    virtual void SwapBuffers() override;  // RenderTarget

    virtual void Clear(const Color& color) override;  // Render Target

    virtual bool IsVisible() override;

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

    virtual void OnWindowResized(const math::ivec2& size) override;

    virtual void OnAppWillEnterBackground() override;
    virtual void OnAppDidEnterBackground() override;
    virtual void OnAppWillEnterForeground() override;
    virtual void OnAppDidEnterForeground() override;

private:
    SDL_Window* m_window;

    VkSurfaceKHR m_surface;

    QueueParameters m_graphics_queue;
    QueueParameters m_present_queue;

    SwapChainParameters m_swapchain;
    VkPipeline m_graphics_pipeline;

    VkCommandPool m_graphics_queue_cmd_pool;

    VkRenderPass m_render_pass;

    Vk_Buffer m_vertex_buffer;
    std::vector<RenderingResourcesData> m_render_resources;
};

}  // namespace engine
