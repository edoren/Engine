#include <System/LogManager.hpp>
#include <System/StringView.hpp>

#include "Vk_Context.hpp"
#include "Vk_Surface.hpp"

namespace engine {

namespace {

const StringView sTag("Vk_Surface");

}  // namespace

Vk_Surface::Vk_Surface() : m_handle(VK_NULL_HANDLE) {}

Vk_Surface::~Vk_Surface() {
    if (m_handle) {
        destroy();
    }
}

bool Vk_Surface::create(SDL_Window* window) {
    Vk_Context& context = Vk_Context::GetInstance();
    VkInstance& instance = context.getVulkanInstance();

    SDL_bool result = SDL_Vulkan_CreateSurface(window, instance, &m_handle);
    if (!result) {
        LogError(sTag, SDL_GetError());
        return false;
    }

    return m_handle != VK_NULL_HANDLE;
}

void Vk_Surface::destroy() {
    Vk_Context& context = Vk_Context::GetInstance();
    VkInstance& instance = context.getVulkanInstance();
    if (instance && m_handle) {
        vkDestroySurfaceKHR(instance, m_handle, nullptr);
        m_handle = VK_NULL_HANDLE;
    }
}

VkSurfaceKHR& Vk_Surface::getHandle() {
    return m_handle;
}

}  // namespace engine
