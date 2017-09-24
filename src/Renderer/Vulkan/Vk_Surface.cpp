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

bool Vk_Surface::Create(SDL_Window* m_window) {
    SDL_SysWMinfo wminfo;
    SDL_VERSION(&wminfo.version);
    if (!SDL_GetWindowWMInfo(m_window, &wminfo)) {
        LogError(sTag, "Error on SDL_GetWindowWMInfo.");
        return false;
    }

    VkResult result = VK_SUCCESS;

    Vk_Context& context = Vk_Context::GetInstance();
    VkInstance& instance = context.GetVulkanInstance();

#if defined(VK_USE_PLATFORM_WIN32_KHR)
    if (wminfo.subsystem == SDL_SYSWM_WINDOWS) {
        VkWin32SurfaceCreateInfoKHR create_info = {
            VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,  // sType
            nullptr,                                          // pNext
            VkWin32SurfaceCreateFlagsKHR(),                   // flags
            GetModuleHandle(nullptr),                         // hinstance
            wminfo.info.win.window                            // hwnd
        };
        result =
            vkCreateWin32SurfaceKHR(instance, &create_info, nullptr, &m_handle);
    }
#elif defined(VK_USE_PLATFORM_XCB_KHR)
    if (wminfo.subsystem == SDL_SYSWM_X11) {
        VkXcbSurfaceCreateInfoKHR create_info = {
            VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR,     // sType
            nullptr,                                           // pNext
            VkXcbSurfaceCreateFlagsKHR(),                      // flags
            XGetXCBConnection(wminfo.info.x11.display),        // connection
            static_cast<xcb_window_t>(wminfo.info.x11.window)  // window
        };
        result =
            vkCreateXcbSurfaceKHR(instance, &create_info, nullptr, &m_handle);
    }
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
    if (wminfo.subsystem == SDL_SYSWM_X11) {
        VkXlibSurfaceCreateInfoKHR create_info = {
            VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,  // sType
            nullptr,                                         // pNext
            VkXlibSurfaceCreateFlagsKHR(),                   // flags
            wminfo.info.x11.display,                         // dpy
            wminfo.info.x11.window                           // window
        };
        result =
            vkCreateXlibSurfaceKHR(instance, &create_info, nullptr, &m_handle);
    }
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
    if (wminfo.subsystem == SDL_SYSWM_ANDROID) {
        VkAndroidSurfaceCreateInfoKHR create_info = {
            VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR,  // sType
            nullptr,                                            // pNext
            VkAndroidSurfaceCreateFlagsKHR(),                   // flags
            wminfo.info.android.window,                         // window
        };
        result = vkCreateAndroidSurfaceKHR(instance, &create_info, nullptr,
                                           &m_handle);
    }
#else
#error "Unsupported Vulkan subsystem"
#endif

    return result == VK_SUCCESS;
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

}  // engine
