#include <string.h>

#include "SDL_vulkan.hpp"
#include <SDL_syswm.h>

namespace SDL {

static SDL_bool SetNames(unsigned* count, const char** names, unsigned inCount, const char* const* inNames) {
    unsigned capacity = *count;
    *count = inCount;
    if (names) {
        if (capacity < inCount) {
            SDL_SetError("Insufficient capacity for extension names: %u < %u", capacity, inCount);
            return SDL_FALSE;
        }
        for (unsigned i = 0; i < inCount; ++i)
            names[i] = inNames[i];
    }
    return SDL_TRUE;
}

SDL_bool GetVulkanInstanceExtensions(std::vector<const char*>& out) {
    const char* driver = SDL_GetCurrentVideoDriver();
    if (!driver) {
        SDL_SetError("No video driver - has SDL_Init(SDL_INIT_VIDEO) been called?");
        return SDL_FALSE;
    }

    out.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

#if defined(VK_KHR_win32_surface)
    if (!strcmp(driver, "windows")) {
        out.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
        return SDL_TRUE;
    }
#endif
#if defined(VK_KHR_xcb_surface)
    if (!strcmp(driver, "x11")) {
        out.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
        return SDL_TRUE;
    }
#endif

    SDL_SetError("Unsupported video driver '%s'", driver);
    return SDL_FALSE;
}

SDL_bool CreateVulkanSurface(SDL_Window* window, VkInstance instance, VkSurfaceKHR* surface) {
    if (!window) {
        SDL_SetError("'window' is null");
        return SDL_FALSE;
    }
    if (instance == VK_NULL_HANDLE) {
        SDL_SetError("'instance' is null");
        return SDL_FALSE;
    }

    SDL_SysWMinfo wminfo;
    SDL_VERSION(&wminfo.version);
    if (!SDL_GetWindowWMInfo(window, &wminfo))
        return SDL_FALSE;

    VkResult result;

    switch (wminfo.subsystem) {
#if defined(VK_KHR_win32_surface)
        case SDL_SYSWM_WINDOWS:
        {
            VkWin32SurfaceCreateInfoKHR createInfo;
            createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
            createInfo.pNext = NULL;
            createInfo.flags = 0;
            createInfo.hinstance = GetModuleHandle(NULL);
            createInfo.hwnd = wminfo.info.win.window;

            result = vkCreateWin32SurfaceKHR(instance, &createInfo, NULL, surface);
            break;
        }
#endif
#if defined(VK_KHR_xcb_surface)
        case SDL_SYSWM_X11:
        {
            VkXcbSurfaceCreateInfoKHR createInfo;
            createInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
            createInfo.pNext = NULL;
            createInfo.flags = 0;
            createInfo.connection = XGetXCBConnection(wminfo.info.x11.display);
            createInfo.window = wminfo.info.x11.window;

            result = vkCreateXcbSurfaceKHR(instance, &createInfo, NULL, surface);
            break;
        }
#endif
        default:
        {
            SDL_SetError("Unsupported subsystem %i", (int)wminfo.subsystem);
            return SDL_FALSE;
        }
    }

    if (result != VK_SUCCESS) {
        SDL_SetError("vkCreate_SurfaceKHR failed.");
        return SDL_FALSE;
    }

    return SDL_TRUE;
}

}
