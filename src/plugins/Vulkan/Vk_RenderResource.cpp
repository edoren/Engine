#include "Vk_RenderResource.hpp"

#include "Vk_Context.hpp"
#include "Vk_Utilities.hpp"

namespace engine {

Vk_RenderResource::Vk_RenderResource()
      : framebuffer(VK_NULL_HANDLE),
        command_buffer(VK_NULL_HANDLE),
        image_available_semaphore(VK_NULL_HANDLE),
        finished_rendering_semaphore(VK_NULL_HANDLE),
        fence(VK_NULL_HANDLE) {}

Vk_RenderResource::~Vk_RenderResource() {
    Destroy();
}

bool Vk_RenderResource::Create() {
    Vk_Context& context = Vk_Context::GetInstance();

    if (!Vk_Utilities::AllocateCommandBuffers(context.GetGraphicsQueueCmdPool(),
                                              1, &command_buffer) ||
        !Vk_Utilities::CreateVulkanSemaphore(&image_available_semaphore) ||
        !Vk_Utilities::CreateVulkanSemaphore(&finished_rendering_semaphore) ||
        !Vk_Utilities::CreateVulkanFence(VK_FENCE_CREATE_SIGNALED_BIT,
                                         &fence)) {
        return false;
    }

    return true;
}

void Vk_RenderResource::Destroy() {
    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.GetVulkanDevice();

    if (framebuffer) {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }
    if (command_buffer) {
        vkFreeCommandBuffers(device, context.GetGraphicsQueueCmdPool(), 1,
                             &command_buffer);
    }
    if (image_available_semaphore) {
        vkDestroySemaphore(device, image_available_semaphore, nullptr);
    }
    if (finished_rendering_semaphore) {
        vkDestroySemaphore(device, finished_rendering_semaphore, nullptr);
    }
    if (fence) {
        vkDestroyFence(device, fence, nullptr);
    }
}

}  // namespace engine
