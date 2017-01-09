#pragma once

#include <vector>

#include "Platform.hpp"

#if (PLATFORM == PLATFORM_WINDOWS)
#define VK_USE_PLATFORM_WIN32_KHR
#include <windows.h>
#elif (PLATFORM == PLATFORM_LINUX)
#define VK_USE_PLATFORM_XCB_KHR
#include <X11/Xlib-xcb.h>
#endif

#include <SDL_video.h>
#include <vulkan/vulkan.hpp>

namespace SDL {

    SDL_bool GetVulkanInstanceExtensions(std::vector<const char*>& out);
    SDL_bool CreateVulkanSurface(SDL_Window* window, VkInstance instance, VkSurfaceKHR* surface);

}
