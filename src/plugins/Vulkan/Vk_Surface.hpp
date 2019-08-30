#pragma once

#include "Vk_Config.hpp"
#include "Vk_Dependencies.hpp"

#include "Vk_VulkanParameters.hpp"

namespace engine {

class VULKAN_PLUGIN_API Vk_Surface {
public:
    Vk_Surface();
    ~Vk_Surface();

    bool Create(SDL_Window* window);
    void Destroy();

    VkSurfaceKHR& GetHandle();

private:
    VkSurfaceKHR m_handle;
};

}  // namespace engine
