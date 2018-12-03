#include <System/LogManager.hpp>

#include "Vk_Context.hpp"
#include "Vk_Surface.hpp"

namespace engine {

namespace {

const String sTag("Vk_Surface");

}  // namespace

Vk_Surface::Vk_Surface() : m_handle(VK_NULL_HANDLE) {}

Vk_Surface::~Vk_Surface() {
    if (m_handle) {
        Destroy();
    }
}

bool Vk_Surface::Create(SDL_Window* window) {
    Vk_Context& context = Vk_Context::GetInstance();
    VkInstance& instance = context.GetVulkanInstance();

    SDL_bool result = SDL_Vulkan_CreateSurface(window, instance, &m_handle);
    if (!result) {
        LogError(sTag, SDL_GetError());
        return false;
    }

    return m_handle != VK_NULL_HANDLE;
}

void Vk_Surface::Destroy() {
    Vk_Context& context = Vk_Context::GetInstance();
    VkInstance& instance = context.GetVulkanInstance();
    if (instance && m_handle) {
        vkDestroySurfaceKHR(instance, m_handle, nullptr);
        m_handle = VK_NULL_HANDLE;
    }
}

VkSurfaceKHR& Vk_Surface::GetHandle() {
    return m_handle;
}

}  // namespace engine
