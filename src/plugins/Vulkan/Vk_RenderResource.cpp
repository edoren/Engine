#include "Vk_RenderResource.hpp"

#include "Vk_Context.hpp"
#include "Vk_Utilities.hpp"

namespace engine {

Vk_RenderResource::Vk_RenderResource()
      : framebuffer(VK_NULL_HANDLE),
        commandBuffer(VK_NULL_HANDLE),
        imageAvailableSemaphore(VK_NULL_HANDLE),
        finishedRenderingSemaphore(VK_NULL_HANDLE),
        fence(VK_NULL_HANDLE) {}

Vk_RenderResource::~Vk_RenderResource() {
    destroy();
}

bool Vk_RenderResource::create() {
    Vk_Context& context = Vk_Context::GetInstance();

    return !(!Vk_Utilities::AllocateCommandBuffers(context.getGraphicsQueueCmdPool(), 1, &commandBuffer) ||
             !Vk_Utilities::CreateVulkanSemaphore(&imageAvailableSemaphore) ||
             !Vk_Utilities::CreateVulkanSemaphore(&finishedRenderingSemaphore) ||
             !Vk_Utilities::CreateVulkanFence(VK_FENCE_CREATE_SIGNALED_BIT, &fence));
}

void Vk_RenderResource::destroy() {
    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.getVulkanDevice();

    if (framebuffer) {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }
    if (commandBuffer) {
        vkFreeCommandBuffers(device, context.getGraphicsQueueCmdPool(), 1, &commandBuffer);
    }
    if (imageAvailableSemaphore) {
        vkDestroySemaphore(device, imageAvailableSemaphore, nullptr);
    }
    if (finishedRenderingSemaphore) {
        vkDestroySemaphore(device, finishedRenderingSemaphore, nullptr);
    }
    if (fence) {
        vkDestroyFence(device, fence, nullptr);
    }
}

}  // namespace engine
