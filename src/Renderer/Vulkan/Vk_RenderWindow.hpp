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
#include "Vk_Surface.hpp"
#include "Vk_SwapChain.hpp"
#include "Vk_VulkanParameters.hpp"
#include "Vk_Image.hpp"

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

    void Draw(Drawable& drawable) override;  // RenderTarget

    void SetUniformBufferObject(const UniformBufferObject& ubo) override;

    void SetBuffers(const Vk_Buffer* vertex_buffer,
                    const Vk_Buffer* index_buffer);

    void AddCommandExecution(Function<void(VkCommandBuffer&)>&& func);

    void SubmitGraphicsCommand(Function<void(VkCommandBuffer&)>&& func);

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
    bool PrepareFrame(VkCommandBuffer command_buffer,
                      Vk_Image& image,
                      VkFramebuffer& framebuffer);

    bool CreateDepthResources();

    bool CreateUniformBuffer();
    bool UpdateUniformBuffer(const UniformBufferObject& ubo);

    bool CreateUBODescriptorPool();
    bool CreateUBODescriptorSetLayout();
    bool AllocateUBODescriptorSet();
    bool UpdateUBODescriptorSet();

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

    SafeQueue<Function<void(VkCommandBuffer&)>> m_command_queue;

    Vk_Image m_depth_image;
    VkFormat m_depth_format;

    Vk_Buffer m_uniform_buffer;
    VkDescriptorPool m_ubo_descriptor_pool;
    VkDescriptorSetLayout m_ubo_descriptor_set_layout;
    VkDescriptorSet m_ubo_descriptor_set;

    std::vector<RenderingResourcesData> m_render_resources;
};

}  // namespace engine
