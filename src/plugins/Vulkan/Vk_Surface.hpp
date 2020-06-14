#pragma once

#include "Vk_Config.hpp"
#include "Vk_Dependencies.hpp"

#include "Vk_VulkanParameters.hpp"

namespace engine {

class VULKAN_PLUGIN_API Vk_Surface {
public:
    Vk_Surface();
    ~Vk_Surface();

    bool create(SDL_Window* window);
    void destroy();

    VkSurfaceKHR& getHandle();

private:
    VkSurfaceKHR m_handle;
};

}  // namespace engine
