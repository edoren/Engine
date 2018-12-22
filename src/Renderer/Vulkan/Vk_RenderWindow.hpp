#pragma once

#include <Util/Prerequisites.hpp>

#include <Math/Math.hpp>
#include <Renderer/Model.hpp>
#include <Renderer/RenderWindow.hpp>
#include <Renderer/Vertex.hpp>
#include <Util/Function.hpp>
#include <Util/SafeQueue.hpp>

#include "Vk_Buffer.hpp"
#include "Vk_Config.hpp"
#include "Vk_Context.hpp"
#include "Vk_Dependencies.hpp"
#include "Vk_Image.hpp"
#include "Vk_Surface.hpp"
#include "Vk_SwapChain.hpp"
#include "Vk_VulkanParameters.hpp"

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

class String;

class Vk_TextureManager;

class VULKAN_PLUGIN_API Vk_RenderWindow : public RenderWindow {
    using CommandType =
        Function<void(uint32, VkCommandBuffer&, VkPipelineLayout&),
                 LAMBDA_FUNCTION_SIZE(16)>;

public:
    Vk_RenderWindow();

    ~Vk_RenderWindow();

    bool Create(const String& name, const math::ivec2& size) override;

    void Destroy(void) override;

    void Reposition(int left, int top) override;

    void Resize(int width, int height) override;

    void SetFullScreen(bool fullscreen, bool is_fake) override;

    void SetVSyncEnabled(bool vsync) override;

    void SwapBuffers() override;  // RenderTarget

    void Clear(const Color& color) override;  // RenderTarget

    bool IsVisible() override;

    void AddCommandExecution(CommandType&& func);

    void SubmitGraphicsCommand(Function<void(VkCommandBuffer&)>&& func);

protected:
    void UpdateProjectionMatrix() override;

private:
    bool CheckWSISupport();

    bool CreateVulkanRenderPass();
    bool CreateVulkanPipeline();

    bool AllocateVulkanCommandBuffers(VkCommandPool& cmd_pool, uint32_t count,
                                      VkCommandBuffer* command_buffer);
    bool CreateVulkanSemaphore(VkSemaphore* semaphore);
    bool CreateVulkanFence(VkFenceCreateFlags flags, VkFence* fence);
    bool CreateRenderingResources();

    bool CreateVulkanFrameBuffer(VkFramebuffer& framebuffer,
                                 VkImageView& image_view);
    bool PrepareFrame(VkCommandBuffer command_buffer, Vk_Image& image,
                      VkFramebuffer& framebuffer);

    bool CreateDepthResources();

    virtual void OnWindowResized(const math::ivec2& size) override;

    virtual void OnAppWillEnterBackground() override;
    virtual void OnAppDidEnterBackground() override;
    virtual void OnAppWillEnterForeground() override;
    virtual void OnAppDidEnterForeground() override;

private:
    SDL_Window* m_window;

    Vk_Surface m_surface;

    QueueParameters* m_graphics_queue;
    QueueParameters* m_present_queue;

    Vk_SwapChain m_swapchain;

    VkPipeline m_graphics_pipeline;
    VkPipelineLayout m_pipeline_layout;

    VkRenderPass m_render_pass;

    SafeQueue<CommandType> m_command_work_queue;

    Vk_Image m_depth_image;
    VkFormat m_depth_format;

    std::vector<RenderingResourcesData> m_render_resources;
};

}  // namespace engine
