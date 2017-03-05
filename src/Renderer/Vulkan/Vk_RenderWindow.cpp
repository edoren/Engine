#include <System/LogManager.hpp>
#include <System/StringFormat.hpp>

#include "Vk_Dependencies.hpp"
#include "Vk_RenderWindow.hpp"

namespace engine {

namespace {

bool CheckExtensionAvailability(
    const char* str, const std::vector<vk::ExtensionProperties>& vec) {
    for (size_t i = 0; i < vec.size(); i++) {
        if (!strcmp(str, vec[i].extensionName)) {
            return true;
        }
    }
    return false;
}

bool CheckLayerAvailability(const char* str,
                            const std::vector<vk::LayerProperties>& vec) {
    for (size_t i = 0; i < vec.size(); i++) {
        if (!strcmp(str, vec[i].layerName)) {
            return true;
        }
    }
    return false;
}

}  // namespace

Vk_RenderWindow::Vk_RenderWindow() : m_window(nullptr) {}

Vk_RenderWindow::~Vk_RenderWindow() {
    Destroy();
}

bool Vk_RenderWindow::Create(const String& name, const math::ivec2& size) {
    // Create the window
    math::ivec2 initial_pos(SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    Uint32 window_flags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;
    m_window = SDL_CreateWindow(name.GetData(), initial_pos.x, initial_pos.y,
                                size.x, size.y, window_flags);
    if (!m_window) {
        String error = String("SDL_CreateWindow fail: ") + SDL_GetError();
        LogError("Vk_RenderWindow", error);
        return false;
    }

    SDL_GetWindowSize(m_window, &m_size.x, &m_size.y);

    // Update the base class attributes
    m_name = name;
    m_size = size;

    // Add the required validation layers
    m_validation_layers.push_back("VK_LAYER_LUNARG_standard_validation");

    // Add the required Instance extensions
    m_instance_extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    m_instance_extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_XCB_KHR)
    m_instance_extensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
    m_instance_extensions.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#endif
    if (m_validation_layers_enabled) {
        m_instance_extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    }

    // Add the required Device extensions
    m_device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    CreateVulkanInstance();
    CreateVulkanSurface();
    CreateVulkanDevice();

    // vk::Queue q, q2;
    // m_device.getQueue(m_graphics_queue_family_index, 0, &q);
    // m_device.getQueue(m_present_queue_family_index, 0, &q2);

    return true;
}

void Vk_RenderWindow::Destroy() {
    m_device.waitIdle();
    m_device.destroy(nullptr);

    m_instance.destroySurfaceKHR(m_surface, nullptr);

    m_instance.destroy(nullptr);

    if (m_window) {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }
}

void Vk_RenderWindow::Reposition(int left, int top) {
    if (m_window) {
        // TODO check errors
        SDL_SetWindowPosition(m_window, left, top);
    }
}

void Vk_RenderWindow::Resize(int width, int height) {
    // TODO check errors
    if (m_window && !IsFullScreen()) {
        SDL_SetWindowSize(m_window, width, height);

        // Update the base class attributes
        // TMP Update the ViewPort
        SDL_GetWindowSize(m_window, &m_size.x, &m_size.y);
        // GL_CALL(glViewport(0, 0, m_size.x, m_size.y));
    }
}

void Vk_RenderWindow::SetFullScreen(bool fullscreen, bool is_fake) {
    // TODO check errors
    if (m_window) {
        m_is_fullscreen = fullscreen;
        Uint32 flag = 0;
        if (fullscreen) {
            flag = (is_fake) ? SDL_WINDOW_FULLSCREEN_DESKTOP
                             : SDL_WINDOW_FULLSCREEN;
        }
        SDL_SetWindowFullscreen(m_window, flag);

        // Update the base class attributes
        // TMP Update the ViewPort
        SDL_GetWindowSize(m_window, &m_size.x, &m_size.y);
        // GL_CALL(glViewport(0, 0, m_size.x, m_size.y));
    }
}

void Vk_RenderWindow::SetVSyncEnabled(bool vsync) {
    // if (SDL_GL_SetSwapInterval(vsync ? 1 : 0) == 0) {
    //     m_is_vsync_enable = vsync;
    // } else {
    //     m_is_vsync_enable = false;
    // }
}

void Vk_RenderWindow::SwapBuffers() {
    // SDL_GL_SwapWindow(m_window);
    // RenderWindow::SwapBuffers();
}

void Vk_RenderWindow::Clear(const Color& color) {  // RenderTarget
    // GL_CALL(glClearColor(color.r, color.g, color.b, color.a));
    // GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}

bool Vk_RenderWindow::IsVisible() {
    Uint32 flags = SDL_WINDOW_HIDDEN | SDL_WINDOW_MINIMIZED;
    Uint32 mask = SDL_GetWindowFlags(m_window);
    return (mask & flags) == 0;
}

bool Vk_RenderWindow::CreateVulkanInstance() {
    if (m_validation_layers_enabled && !CheckVulkanValidationLayerSupport()) {
        LogFatal("Vk_RenderWindow",
                 "Validation layers requested, but not available");
        return false;
    }

    // Define the application information
    vk::ApplicationInfo appInfo{
        "Hello Vulkan",            // pApplicationName
        VK_MAKE_VERSION(1, 0, 0),  // applicationVersion
        "Engine",                  // pEngineName
        VK_MAKE_VERSION(1, 0, 0),  // engineVersion
        VK_API_VERSION_1_0         // apiVersion
    };

    // Get the validation layers
    if (!CheckVulkanValidationLayerSupport()) {
        LogFatal("Vk_RenderWindow", "Error validation layers");
        return false;
    };

    if (!CheckVulkanInstanceExtensionsSupport()) {
        LogFatal("Vk_RenderWindow", "Error instance extensions");
        return false;
    };

    // Define all the information for the instance
    vk::InstanceCreateInfo create_info{
        vk::InstanceCreateFlags(),                        //  flags
        &appInfo,                                         //  pApplicationInfo
        static_cast<uint32>(m_validation_layers.size()),  //  enabledLayerCount
        m_validation_layers.data(),  //  ppEnabledLayerNames
        static_cast<uint32>(
            m_instance_extensions.size()),  //  enabledExtensionCount
        m_instance_extensions.data()        //  ppEnabledExtensionNames
    };

    // Create the Vulkan instance based on the provided info
    vk::Result result = vk::createInstance(&create_info, nullptr, &m_instance);
    if (result != vk::Result::eSuccess) {
        LogFatal("Vk_RenderWindow", "Failed to create Vulkan instance");
        return false;
    }

    return true;
}

bool Vk_RenderWindow::CreateVulkanSurface() {
    SDL_SysWMinfo wminfo;
    SDL_VERSION(&wminfo.version);
    if (!SDL_GetWindowWMInfo(m_window, &wminfo)) return false;

    vk::Result result;

#if defined(VK_USE_PLATFORM_WIN32_KHR)
    if (wminfo.subsystem == SDL_SYSWM_WINDOWS) {
        vk::Win32SurfaceCreateInfoKHR create_info;
        create_info.hinstance = GetModuleHandle(nullptr);
        create_info.hwnd = wminfo.info.win.window;

        result =
            m_instance.createWin32SurfaceKHR(&create_info, nullptr, &m_surface);
    }
#elif defined(VK_USE_PLATFORM_XCB_KHR)
    if (wminfo.subsystem == SDL_SYSWM_X11) {
        vk::XcbSurfaceCreateInfoKHR create_info;
        create_info.connection = XGetXCBConnection(wminfo.info.x11.display);
        create_info.window = wminfo.info.x11.window;

        result =
            m_instance.createXcbSurfaceKHR(&create_info, nullptr, &m_surface);
    }
#endif
    else {
        LogFatal("Vk_RenderWindow", "Unsupported subsystem");
        return false;
    }

    if (result != vk::Result::eSuccess) {
        LogError("Vk_RenderWindow", "vkCreate_SurfaceKHR failed.");
        return false;
    }

    return true;
}

bool Vk_RenderWindow::CreateVulkanDevice() {
    vk::Result result;

    // Query all the avaliable physical devices
    uint32 physical_devices_count = 0;
    std::vector<vk::PhysicalDevice> physical_devices;
    result =
        m_instance.enumeratePhysicalDevices(&physical_devices_count, nullptr);
    if (physical_devices_count > 0 && result == vk::Result::eSuccess) {
        physical_devices.resize(physical_devices_count);
        result = m_instance.enumeratePhysicalDevices(&physical_devices_count,
                                                     physical_devices.data());
    }
    if (physical_devices_count == 0 || result != vk::Result::eSuccess) {
        LogFatal("Vk_RenderWindow", "Error querying physical devices");
        return false;
    }

    // Check all the queried physical devices for one with the required
    // caracteristics and avaliable queues
    uint32 selected_graphics_queue_family_index = UINT32_MAX;
    uint32 selected_present_queue_family_index = UINT32_MAX;
    vk::PhysicalDevice* selected_physical_device = nullptr;
    for (size_t i = 0; i < physical_devices.size(); i++) {
        if (CheckPhysicalDevice(physical_devices[i],
                                selected_graphics_queue_family_index,
                                selected_present_queue_family_index)) {
            selected_physical_device = &physical_devices[i];
        }
    }
    if (selected_physical_device == nullptr ||
        selected_graphics_queue_family_index == UINT32_MAX ||
        selected_present_queue_family_index == UINT32_MAX) {
        LogFatal(
            "Vk_RenderWindow",
            "No physical device that supports the required caracteristics");
        return false;
    }

    // Define the queue families information
    std::vector<float> queue_priorities = {1.0f};
    std::vector<vk::DeviceQueueCreateInfo> queue_create_infos;
    queue_create_infos.push_back(vk::DeviceQueueCreateInfo{
        vk::DeviceQueueCreateFlags(),                  // flags
        selected_graphics_queue_family_index,          // queueFamilyIndex
        static_cast<uint32>(queue_priorities.size()),  // queueCount
        queue_priorities.data()                        // pQueuePriorities
    });
    if (selected_graphics_queue_family_index !=
        selected_present_queue_family_index) {
        queue_create_infos.push_back(vk::DeviceQueueCreateInfo{
            vk::DeviceQueueCreateFlags(),                  // flags
            selected_present_queue_family_index,           // queueFamilyIndex
            static_cast<uint32>(queue_priorities.size()),  // queueCount
            queue_priorities.data()                        // pQueuePriorities
        });
    }

    // Define all the information for the logical device
    vk::DeviceCreateInfo device_create_info{
        vk::DeviceCreateFlags(),                         // flags
        static_cast<uint32>(queue_create_infos.size()),  // queueCreateInfoCount
        queue_create_infos.data(),                       // pQueueCreateInfos
        static_cast<uint32>(m_validation_layers.size()),  // enabledLayerCount
        m_validation_layers.data(),                       // ppEnabledLayerNames
        0,        // enabledExtensionCount
        nullptr,  // ppEnabledExtensionNames
        nullptr   // pEnabledFeatures
    };

    // Create the logical device based on the retrived info
    result = selected_physical_device->createDevice(&device_create_info,
                                                    nullptr, &m_device);
    if (result != vk::Result::eSuccess) {
        LogFatal("Vk_RenderWindow", "Could not create Vulkan device");
        return false;
    }

    m_graphics_queue_family_index = selected_graphics_queue_family_index;
    m_present_queue_family_index = selected_present_queue_family_index;
    return true;
}

bool Vk_RenderWindow::CheckVulkanValidationLayerSupport() const {
    vk::Result result;

    // Get the avaliable layers
    uint32 layer_count = 0;
    std::vector<vk::LayerProperties> avaliable_layers;
    result = vk::enumerateInstanceLayerProperties(&layer_count, nullptr);
    if (layer_count > 0 && result == vk::Result::eSuccess) {
        avaliable_layers.resize(layer_count);
        result = vk::enumerateInstanceLayerProperties(&layer_count,
                                                      avaliable_layers.data());
    }

    // Check that the avaliable layers could be retreived
    if (layer_count == 0 || result != vk::Result::eSuccess) {
        LogError("Vk_RenderWindow",
                 "Error occurred during validation layers enumeration");
        return false;
    }

    // Check that all the validation layers exists
    for (size_t i = 0; i < m_validation_layers.size(); i++) {
        if (!CheckLayerAvailability(m_validation_layers[i], avaliable_layers)) {
            LogError("Vk_RenderWindow",
                     "Could not find validation layer "
                     "named: {}"_format(m_validation_layers[i]));
            return false;
        }
    }

    return true;
}

bool Vk_RenderWindow::CheckVulkanInstanceExtensionsSupport() const {
    vk::Result result;

    // Get the avaliable extensions
    uint32 extensions_count = 0;
    std::vector<vk::ExtensionProperties> available_extensions;
    result = vk::enumerateInstanceExtensionProperties(
        nullptr, &extensions_count, nullptr);
    if (extensions_count > 0 && result == vk::Result::eSuccess) {
        available_extensions.resize(extensions_count);
        result = vk::enumerateInstanceExtensionProperties(
            nullptr, &extensions_count, available_extensions.data());
    }

    // Check that the avaliable extensions could be retreived
    if (extensions_count == 0 || result != vk::Result::eSuccess) {
        LogError("Vk_RenderWindow",
                 "Error occurred during instance extensions enumeration");
        return false;
    }

    // Check that all the required instance extensions exists
    for (size_t i = 0; i < m_instance_extensions.size(); i++) {
        if (!CheckExtensionAvailability(m_instance_extensions[i],
                                        available_extensions)) {
            LogError("Vk_RenderWindow",
                     "Could not find instance extension "
                     "named: {}"_format(m_instance_extensions[i]));
            return false;
        }
    }

    return true;
}

bool Vk_RenderWindow::CheckPhysicalDevice(
    const vk::PhysicalDevice& physical_device,
    uint32& selected_graphics_queue_family_index,
    uint32& selected_present_queue_family_index) {
    vk::Result result;

    // Check the PhysicalDevice properties and features
    vk::PhysicalDeviceProperties properties;
    vk::PhysicalDeviceFeatures features;
    physical_device.getProperties(&properties);
    physical_device.getFeatures(&features);

    uint32 major_version = VK_VERSION_MAJOR(properties.apiVersion);
    // uint32 minor_version = VK_VERSION_MINOR(properties.apiVersion);
    // uint32 patch_version = VK_VERSION_PATCH(properties.apiVersion);

    if (major_version < 1 && properties.limits.maxImageDimension2D < 4096) {
        LogError("Vk_RenderWindow",
                 "Physical device {} doesn't support required "
                 "parameters"_format(properties.deviceName));
        return false;
    }

    // Check if the physical device support the required extensions
    uint32 extensions_count = 0;
    std::vector<vk::ExtensionProperties> available_extensions;
    result = physical_device.enumerateDeviceExtensionProperties(
        nullptr, &extensions_count, nullptr);
    if (result == vk::Result::eSuccess && extensions_count > 0) {
        available_extensions.resize(extensions_count);
        physical_device.enumerateDeviceExtensionProperties(
            nullptr, &extensions_count, &available_extensions[0]);
    }

    // Check that the avaliable extensions could be retreived
    if (result != vk::Result::eSuccess || extensions_count == 0) {
        LogError("Vk_RenderWindow",
                 "Error occurred during physical device {} extensions "
                 "enumeration"_format(properties.deviceName));
        return false;
    }

    // Check that all the required device extensions exists
    for (size_t i = 0; i < m_device_extensions.size(); i++) {
        if (!CheckExtensionAvailability(m_device_extensions[i],
                                        available_extensions)) {
            LogError("Vk_RenderWindow",
                     "Physical device {} doesn't support extension "
                     "named \"{}\""_format(properties.deviceName,
                                           m_device_extensions[i]));
            return false;
        }
    }

    // Retreive all the queue families properties
    uint32 queue_families_count = 0;
    physical_device.getQueueFamilyProperties(&queue_families_count, nullptr);
    if (queue_families_count == 0) {
        LogError("Vk_RenderWindow",
                 "Physical device {} doesn't have any queue "
                 "families"_format(properties.deviceName));
        return false;
    }
    std::vector<vk::QueueFamilyProperties> queue_family_properties(
        queue_families_count);
    physical_device.getQueueFamilyProperties(&queue_families_count,
                                             queue_family_properties.data());

    std::vector<vk::Bool32> queue_present_support(queue_families_count);

    // Find a queue family that supports graphics queue and other that supports
    // present queue
    uint32 graphics_queue_family_index = UINT32_MAX;
    uint32 present_queue_family_index = UINT32_MAX;
    for (uint32 i = 0; i < queue_families_count; i++) {
        if (queue_family_properties[i].queueCount > 0 &&
            queue_family_properties[i].queueFlags &
                vk::QueueFlagBits::eGraphics) {
            // Retreive the present support in the queue
            physical_device.getSurfaceSupportKHR(i, m_surface,
                                                 &queue_present_support[i]);

            // Select first queue that supports graphics
            if (graphics_queue_family_index == UINT32_MAX) {
                graphics_queue_family_index = i;
            }

            // If there is queue that supports both graphics and present
            // prefer it
            if (queue_present_support[i]) {
                selected_graphics_queue_family_index = i;
                selected_present_queue_family_index = i;
                return true;
            }
        }
    }

    // We don't have queue that supports both graphics and present so we have
    // to use separate queues
    for (uint32 i = 0; i < queue_families_count; i++) {
        if (queue_present_support[i]) {
            present_queue_family_index = i;
            break;
        }
    }

    // If this device doesn't support queues with graphics and present
    // capabilities don't use it
    if (graphics_queue_family_index == UINT32_MAX ||
        present_queue_family_index == UINT32_MAX) {
        LogError("Vk_RenderWindow",
                 "Could not find queue family with required properties "
                 "on physical device: {}"_format(properties.deviceName));
        return false;
    }

    selected_graphics_queue_family_index = graphics_queue_family_index;
    selected_present_queue_family_index = present_queue_family_index;
    return true;
}

}  // namespace engine
