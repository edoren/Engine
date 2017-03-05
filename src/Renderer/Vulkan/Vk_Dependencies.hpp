#pragma once

#include <Util/Platform.hpp>

#if (PLATFORM == PLATFORM_WINDOWS)
#define VK_USE_PLATFORM_WIN32_KHR
#include <windows.h>
#elif (PLATFORM == PLATFORM_LINUX)
#define VK_USE_PLATFORM_XCB_KHR
#include <X11/Xlib-xcb.h>
// #define VK_USE_PLATFORM_WAYLAND_KHR
// #define VK_USE_PLATFORM_MIR_KHR
#elif PLATFORM_IS(PLATFORM_ANDROID)
#define VK_USE_PLATFORM_ANDROID_KHR
#endif

#define VULKAN_HPP_DISABLE_ENHANCED_MODE
#include <vulkan/vulkan.hpp>

#include <SDL.h>
#include <SDL_syswm.h>
