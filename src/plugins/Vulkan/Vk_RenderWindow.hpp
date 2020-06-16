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
#include "Vk_RenderResource.hpp"
#include "Vk_Surface.hpp"
#include "Vk_SwapChain.hpp"
#include "Vk_VulkanParameters.hpp"

namespace engine {

class String;

class Vk_TextureManager;

class VULKAN_PLUGIN_API Vk_RenderWindow : public RenderWindow {
    using CommandType = Function<void(uint32, VkCommandBuffer&, VkPipelineLayout&), LAMBDA_FUNCTION_SIZE(20)>;

public:
    Vk_RenderWindow();

    ~Vk_RenderWindow() override;

    bool create(const String& name, const math::ivec2& size) override;

    void destroy() override;

    void resize(int width, int height) override;

    void setFullScreen(bool fullscreen, bool isFake) override;

    void setVSyncEnabled(bool vsync) override;

    void swapBuffers() override;  // RenderTarget

    void clear(const Color& color) override;  // RenderTarget

    void addCommandExecution(CommandType&& func);

    void submitGraphicsCommand(Function<void(VkCommandBuffer&)>&& func);

protected:
    void updateProjectionMatrix() override;

private:
    bool checkWsiSupport();

    bool createVulkanRenderPass();
    bool createVulkanPipeline();

    bool createVulkanSemaphore(VkSemaphore* semaphore);
    bool createVulkanFence(VkFenceCreateFlags flags, VkFence* fence);
    bool createRenderingResources();

    bool createVulkanFrameBuffer(VkFramebuffer& framebuffer, VkImageView& imageView);
    bool prepareFrame(VkCommandBuffer commandBuffer, Vk_Image& image, VkFramebuffer& framebuffer);

    bool createDepthResources();

    void onWindowResized(const math::ivec2& size) override;

    void onAppWillEnterBackground() override;
    void onAppDidEnterBackground() override;
    void onAppWillEnterForeground() override;
    void onAppDidEnterForeground() override;

    Vk_Surface m_surface;

    QueueParameters* m_graphicsQueue;
    QueueParameters* m_presentQueue;

    Vk_SwapChain m_swapchain;

    VkPipeline m_graphicsPipeline;
    VkPipelineLayout m_pipelineLayout;

    VkRenderPass m_renderPass;

    SafeQueue<CommandType> m_commandWorkQueue;

    Vk_Image m_depthImage;
    VkFormat m_depthFormat;

    std::vector<Vk_RenderResource> m_renderResources;
};

}  // namespace engine
