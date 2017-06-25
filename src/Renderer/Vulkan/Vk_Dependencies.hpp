#pragma once

#include <Util/Platform.hpp>

#if PLATFORM_IS(PLATFORM_WINDOWS)
    #define VK_USE_PLATFORM_WIN32_KHR
    #include <windows.h>
#elif PLATFORM_IS(PLATFORM_LINUX)
    #define VK_USE_PLATFORM_XCB_KHR
    #include <X11/Xlib-xcb.h>
    // #define VK_USE_PLATFORM_WAYLAND_KHR
    // #define VK_USE_PLATFORM_MIR_KHR
#elif PLATFORM_IS(PLATFORM_ANDROID)
    #define VK_USE_PLATFORM_ANDROID_KHR
#endif

#define VULKAN_HPP_NO_SMART_HANDLE
#define VULKAN_HPP_DISABLE_ENHANCED_MODE
#include <vulkan/vulkan.hpp>

#include <SDL.h>
#include <SDL_syswm.h>

// On Windows undefine this anoying macros defined by windows.h
#if PLATFORM_IS(PLATFORM_WINDOWS)
    #undef ERROR
    #undef TRANSPARENT
#endif
