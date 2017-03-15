#include <System/LogManager.hpp>
#include <System/StringFormat.hpp>

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

Vk_RenderWindow::Vk_RenderWindow()
      : m_window(nullptr), m_validation_layers_enabled(true) {}

Vk_RenderWindow::~Vk_RenderWindow() {
    Destroy();
}

bool Vk_RenderWindow::Create(const String& name, const math::ivec2& size) {
    // Create the window
    math::ivec2 initial_pos(SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    uint32 window_flags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;
    m_window = SDL_CreateWindow(name.GetData(), initial_pos.x, initial_pos.y,
                                size.x, size.y, window_flags);
    if (!m_window) {
        LogError("Vk_RenderWindow",
                 "SDL_CreateWindow fail: "_format(SDL_GetError()));
        return false;
    }

    // Update the base class attributes
    SDL_GetWindowSize(m_window, &m_size.x, &m_size.y);
    m_name = name;
    m_size = size;

    // Add the required validation layers
    if (m_validation_layers_enabled) {
        m_validation_layers.push_back("VK_LAYER_LUNARG_standard_validation");
    }

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

    // Check the validation layers support
    if (m_validation_layers_enabled && !CheckVulkanValidationLayerSupport()) {
        LogFatal("Vk_RenderWindow",
                 "Validation layers requested, but not available");
        return false;
    }

    CreateVulkanInstance();
    CreateVulkanSurface();
    CreateVulkanDevice();
    CreateVulkanSemaphores();
    CreateVulkanQueues();
    CreateVulkanSwapChain();
    CreateVulkanCommandBuffers();

    return true;
}

void Vk_RenderWindow::Destroy() {
    CleanCommandBuffers();

    if (m_device) {
        m_device.waitIdle();

        if (m_image_avaliable_semaphore) {
            m_device.destroySemaphore(m_image_avaliable_semaphore, nullptr);
        }
        if (m_rendering_finished_semaphore) {
            m_device.destroySemaphore(m_rendering_finished_semaphore, nullptr);
        }
        if (m_swapchain) {
            m_device.destroySwapchainKHR(m_swapchain, nullptr);
        }

        m_device.destroy(nullptr);
    }

    if (m_instance) {
        if (m_surface) {
            m_instance.destroySurfaceKHR(m_surface, nullptr);
        }
        m_instance.destroy(nullptr);
    }

    if (m_window) {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }

    m_image_avaliable_semaphore = vk::Semaphore();
    m_rendering_finished_semaphore = vk::Semaphore();
    m_swapchain = vk::SwapchainKHR();
    m_device = vk::Device();
    m_surface = vk::SurfaceKHR();
    m_instance = vk::Instance();
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
        OnWindowSizeChanged();
    }
}

void Vk_RenderWindow::SetFullScreen(bool fullscreen, bool is_fake) {
    // TODO check errors
    if (m_window) {
        m_is_fullscreen = fullscreen;
        uint32 flag = 0;
        if (fullscreen) {
            flag = (is_fake) ? SDL_WINDOW_FULLSCREEN_DESKTOP
                             : SDL_WINDOW_FULLSCREEN;
        }
        SDL_SetWindowFullscreen(m_window, flag);
        OnWindowSizeChanged();
    }
}

void Vk_RenderWindow::SetVSyncEnabled(bool /*vsync*/) {
    // if (SDL_GL_SetSwapInterval(vsync ? 1 : 0) == 0) {
    //     m_is_vsync_enable = vsync;
    // } else {
    //     m_is_vsync_enable = false;
    // }
}

void Vk_RenderWindow::SwapBuffers() {
    vk::Result result;

    uint32 image_index = 0;
    result = m_device.acquireNextImageKHR(m_swapchain, UINT64_MAX,
                                          m_image_avaliable_semaphore,
                                          vk::Fence(), &image_index);
    switch (result) {
        case vk::Result::eSuccess:
        case vk::Result::eSuboptimalKHR:
            break;
        case vk::Result::eErrorOutOfDateKHR:
            OnWindowSizeChanged();
            return;
        default:
            LogError("Vk_RenderWindow",
                     "Problem occurred during swap chain image acquisition");
            return;
    }

    vk::PipelineStageFlags wait_dst_stage_mask =
        vk::PipelineStageFlagBits::eTransfer;
    vk::SubmitInfo submit_info{
        1,                                          // waitSemaphoreCount
        &m_image_avaliable_semaphore,               // pWaitSemaphores
        &wait_dst_stage_mask,                       // pWaitDstStageMask;
        1,                                          // commandBufferCount
        &m_present_queue_cmd_buffers[image_index],  // pCommandBuffers
        1,                                          // signalSemaphoreCount
        &m_rendering_finished_semaphore             // pSignalSemaphores
    };

    result = m_present_queue.submit(1, &submit_info, vk::Fence());
    if (result != vk::Result::eSuccess) {
        LogError("Vk_RenderWindow", "Error submitting the command buffers");
        return;
    }

    vk::PresentInfoKHR present_info{
        1,                                // waitSemaphoreCount
        &m_rendering_finished_semaphore,  // pWaitSemaphores
        1,                                // swapchainCount
        &m_swapchain,                     // pSwapchains
        &image_index,                     // pImageIndices
        nullptr                           // pResults
    };

    result = m_present_queue.presentKHR(&present_info);
    switch (result) {
        case vk::Result::eSuccess:
            break;
        case vk::Result::eErrorOutOfDateKHR:
        case vk::Result::eSuboptimalKHR:
            OnWindowSizeChanged();
            return;
        default:
            LogError("Vk_RenderWindow",
                     "Problem occurred during image presentation");
            return;
    }
}

void Vk_RenderWindow::Clear(const Color& /*color*/) {  // RenderTarget
    // GL_CALL(glClearColor(color.r, color.g, color.b, color.a));
    // GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}

bool Vk_RenderWindow::IsVisible() {
    uint32 flags = SDL_WINDOW_HIDDEN | SDL_WINDOW_MINIMIZED;
    uint32 mask = SDL_GetWindowFlags(m_window);
    return (mask & flags) == 0;
}

bool Vk_RenderWindow::CreateVulkanInstance() {
    // Define the application information
    vk::ApplicationInfo appInfo{
        m_name.GetData(),          // pApplicationName
        VK_MAKE_VERSION(1, 0, 0),  // applicationVersion
        "Engine",                  // pEngineName
        VK_MAKE_VERSION(1, 0, 0),  // engineVersion
        VK_API_VERSION_1_0         // apiVersion
    };

    // Check that all the instance extensions are supported
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
        vk::Win32SurfaceCreateInfoKHR create_info{
            vk::Win32SurfaceCreateFlagsKHR(),  // flags
            GetModuleHandle(nullptr),          // hinstance
            wminfo.info.win.window             // hwnd
        };
        result =
            m_instance.createWin32SurfaceKHR(&create_info, nullptr, &m_surface);
    }
#elif defined(VK_USE_PLATFORM_XCB_KHR)
    if (wminfo.subsystem == SDL_SYSWM_X11) {
        vk::XcbSurfaceCreateInfoKHR create_info{
            vk::XcbSurfaceCreateFlagsKHR(),                    // flags
            XGetXCBConnection(wminfo.info.x11.display),        // connection
            static_cast<xcb_window_t>(wminfo.info.x11.window)  // window
        };
        result =
            m_instance.createXcbSurfaceKHR(&create_info, nullptr, &m_surface);
    }
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
    if (wminfo.subsystem == SDL_SYSWM_X11) {
        vk::XlibSurfaceCreateInfoKHR create_info{
            vk::XlibSurfaceCreateFlagsKHR(),  // flags
            wminfo.info.x11.display,          // dpy
            wminfo.info.x11.window            // window
        };
        result =
            m_instance.createXlibSurfaceKHR(&create_info, nullptr, &m_surface);
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
        static_cast<uint32>(
            m_device_extensions.size()),  // enabledExtensionCount
        m_device_extensions.data(),       // ppEnabledExtensionNames
        nullptr                           // pEnabledFeatures
    };

    // Create the logical device based on the retrived info
    result = selected_physical_device->createDevice(&device_create_info,
                                                    nullptr, &m_device);
    if (result != vk::Result::eSuccess) {
        LogFatal("Vk_RenderWindow", "Could not create Vulkan device");
        return false;
    }

    m_physical_device = *selected_physical_device;
    m_graphics_queue_family_index = selected_graphics_queue_family_index;
    m_present_queue_family_index = selected_present_queue_family_index;
    return true;
}

bool Vk_RenderWindow::CreateVulkanSemaphores() {
    vk::SemaphoreCreateInfo info{vk::SemaphoreCreateFlags()};
    vk::Result result1 =
        m_device.createSemaphore(&info, nullptr, &m_image_avaliable_semaphore);
    vk::Result result2 = m_device.createSemaphore(
        &info, nullptr, &m_rendering_finished_semaphore);
    if (result1 != vk::Result::eSuccess || result2 != vk::Result::eSuccess) {
        LogError("Vk_RenderWindow", "Could not create semaphores");
        return false;
    }
    return true;
}

bool Vk_RenderWindow::CreateVulkanSwapChain() {
    vk::Result result;

    // Get the Surface capabilities
    vk::SurfaceCapabilitiesKHR surface_capabilities;
    result = m_physical_device.getSurfaceCapabilitiesKHR(m_surface,
                                                         &surface_capabilities);
    if (result != vk::Result::eSuccess) {
        LogError("Vk_RenderWindow",
                 "Could not check presentation surface capabilities");
        return false;
    }

    // Query all the supported Surface formats
    uint32 formats_count = 0;
    std::vector<vk::SurfaceFormatKHR> surface_formats;
    result = m_physical_device.getSurfaceFormatsKHR(m_surface, &formats_count,
                                                    nullptr);
    if (formats_count > 0 && result == vk::Result::eSuccess) {
        surface_formats.resize(formats_count);
        result = m_physical_device.getSurfaceFormatsKHR(
            m_surface, &formats_count, surface_formats.data());
    }

    // Check that the surface formats where queried successfully
    if (formats_count == 0 || result != vk::Result::eSuccess) {
        LogError("Vk_RenderWindow",
                 "Error occurred during presentation surface formats "
                 "enumeration");
        return false;
    }

    // Query all the supported Surface present modes
    uint32 present_modes_count = 0;
    std::vector<vk::PresentModeKHR> present_modes;
    result = m_physical_device.getSurfacePresentModesKHR(
        m_surface, &present_modes_count, nullptr);
    if (present_modes_count > 0 && result == vk::Result::eSuccess) {
        present_modes.resize(present_modes_count);
        result = m_physical_device.getSurfacePresentModesKHR(
            m_surface, &present_modes_count, present_modes.data());
    }

    // Check that the surface present modes where queried successfully
    if (present_modes_count == 0 || result != vk::Result::eSuccess) {
        LogError("Vk_RenderWindow",
                 "Error occurred during presentation surface formats "
                 "enumeration");
        return false;
    }

    // Retreive all the Swapchain related information
    uint32 desired_number_of_images =
        GetVulkanSwapChainNumImages(surface_capabilities);
    vk::SurfaceFormatKHR desired_format =
        GetVulkanSwapChainFormat(surface_formats);
    vk::Extent2D desired_extent =
        GetVulkanSwapChainExtent(surface_capabilities);
    vk::ImageUsageFlags desired_usage =
        GetVulkanSwapChainUsageFlags(surface_capabilities);
    vk::SurfaceTransformFlagBitsKHR desired_transform =
        GetVulkanSwapChainTransform(surface_capabilities);
    vk::PresentModeKHR desired_present_mode =
        GetVulkanSwapChainPresentMode(present_modes);
    vk::SwapchainKHR old_swap_chain = m_swapchain;

    if (static_cast<int>(VkImageUsageFlags(desired_usage)) == -1) {
        return false;
    }

    vk::SwapchainCreateInfoKHR swap_chain_create_info{
        vk::SwapchainCreateFlagsKHR(),           // flags
        m_surface,                               // surface
        desired_number_of_images,                // minImageCount
        desired_format.format,                   // imageFormat
        desired_format.colorSpace,               // imageColorSpace
        desired_extent,                          // imageExtent
        1,                                       // imageArrayLayers
        desired_usage,                           // imageUsage
        vk::SharingMode::eExclusive,             // imageSharingMode
        0,                                       // queueFamilyIndexCount
        nullptr,                                 // pQueueFamilyIndices
        desired_transform,                       // preTransform
        vk::CompositeAlphaFlagBitsKHR::eOpaque,  // compositeAlpha
        desired_present_mode,                    // presentMode
        true,                                    // clipped
        old_swap_chain                           // oldSwapchain
    };

    result = m_device.createSwapchainKHR(&swap_chain_create_info, nullptr,
                                         &m_swapchain);
    if (result != vk::Result::eSuccess) {
        LogFatal("Vk_RenderWindow", "Could not create swap chain");
        return false;
    }
    if (old_swap_chain) {
        m_device.destroySwapchainKHR(old_swap_chain, nullptr);
    }

    return true;
}

bool Vk_RenderWindow::CreateVulkanQueues() {
    m_device.getQueue(m_graphics_queue_family_index, 0, &m_graphics_queue);
    m_device.getQueue(m_present_queue_family_index, 0, &m_present_queue);
    return true;
}

bool Vk_RenderWindow::CreateVulkanCommandBuffers() {
    vk::Result result;

    // Create the pool for the command buffers
    vk::CommandPoolCreateInfo cmd_pool_create_info = {
        vk::CommandPoolCreateFlags(),  // flags
        m_present_queue_family_index   // queueFamilyIndex
    };
    result = m_device.createCommandPool(&cmd_pool_create_info, nullptr,
                                        &m_present_queue_cmd_pool);
    if (result != vk::Result::eSuccess) {
        LogError("Vk_RenderWindow", "Could not create a command pool");
        return false;
    }

    // Get the number of images in the swapchain
    uint32 image_count = 0;
    result = m_device.getSwapchainImagesKHR(m_swapchain, &image_count, nullptr);
    if (image_count == 0 || result != vk::Result::eSuccess) {
        LogError("Vk_RenderWindow",
                 "Could not get the number of swap chain images");
        return false;
    }

    // Reserve a command buffer for each image
    m_present_queue_cmd_buffers.resize(image_count);

    // Allocate space in the pool for each buffer
    vk::CommandBufferAllocateInfo cmd_buffer_allocate_info{
        m_present_queue_cmd_pool,          // commandPool
        vk::CommandBufferLevel::ePrimary,  // level
        image_count                        // bufferCount
    };
    result = m_device.allocateCommandBuffers(
        &cmd_buffer_allocate_info, m_present_queue_cmd_buffers.data());
    if (result != vk::Result::eSuccess) {
        LogError("Vk_RenderWindow", "Could not allocate command buffers");
        return false;
    }

    // Define some command buffers
    if (!RecordCommandBuffers()) {
        LogError("Vk_RenderWindow", "Could not record command buffers");
        return false;
    }

    return true;
}

bool Vk_RenderWindow::RecordCommandBuffers() {
    vk::Result result;

    uint32 image_count =
        static_cast<uint32>(m_present_queue_cmd_buffers.size());

    std::vector<vk::Image> swap_chain_images(image_count);
    result = m_device.getSwapchainImagesKHR(m_swapchain, &image_count,
                                            swap_chain_images.data());
    if (result != vk::Result::eSuccess) {
        LogError("Vk_RenderWindow", "Could not get swap chain images");
        return false;
    }

    vk::CommandBufferBeginInfo cmd_buffer_begin_info{
        vk::CommandBufferUsageFlagBits::eSimultaneousUse,  // flags
        nullptr                                            // pInheritanceInfo
    };

    vk::ClearColorValue clear_color(
        std::array<float, 4>{0.0f, 0.5451f, 0.5451f, 0.0f});

    vk::ImageSubresourceRange image_subresource_range{
        vk::ImageAspectFlagBits::eColor,  // aspectMask
        0,                                // baseMipLevel
        1,                                // levelCount
        0,                                // baseArrayLayer
        1                                 // layerCount
    };

    for (size_t i = 0; i < m_present_queue_cmd_buffers.size(); i++) {
        vk::ImageMemoryBarrier barrier_from_present_to_clear{
            vk::AccessFlagBits::eMemoryRead,       // srcAccessMask
            vk::AccessFlagBits::eTransferWrite,    // dstAccessMask
            vk::ImageLayout::eUndefined,           // oldLayout
            vk::ImageLayout::eTransferDstOptimal,  // newLayout
            m_present_queue_family_index,          // srcQueueFamilyIndex
            m_present_queue_family_index,          // dstQueueFamilyIndex
            swap_chain_images[i],                  // image
            image_subresource_range                // subresourceRange
        };

        vk::ImageMemoryBarrier barrier_from_clear_to_present{
            vk::AccessFlagBits::eTransferWrite,    // srcAccessMask
            vk::AccessFlagBits::eMemoryRead,       // dstAccessMask
            vk::ImageLayout::eTransferDstOptimal,  // oldLayout
            vk::ImageLayout::ePresentSrcKHR,       // newLayout
            m_present_queue_family_index,          // srcQueueFamilyIndex
            m_present_queue_family_index,          // dstQueueFamilyIndex
            swap_chain_images[i],                  // image
            image_subresource_range                // subresourceRange
        };

        m_present_queue_cmd_buffers[i].begin(&cmd_buffer_begin_info);

        m_present_queue_cmd_buffers[i].pipelineBarrier(
            vk::PipelineStageFlagBits::eTransfer,
            vk::PipelineStageFlagBits::eTransfer, vk::DependencyFlags(), 0,
            nullptr, 0, nullptr, 1, &barrier_from_present_to_clear);

        m_present_queue_cmd_buffers[i].clearColorImage(
            swap_chain_images[i], vk::ImageLayout::eTransferDstOptimal,
            &clear_color, 1, &image_subresource_range);

        m_present_queue_cmd_buffers[i].pipelineBarrier(
            vk::PipelineStageFlagBits::eTransfer,
            vk::PipelineStageFlagBits::eBottomOfPipe, vk::DependencyFlags(), 0,
            nullptr, 0, nullptr, 1, &barrier_from_clear_to_present);

        result = m_present_queue_cmd_buffers[i].end();
        if (result != vk::Result::eSuccess) {
            LogError("Vk_RenderWindow", "Could not record command buffers");
            return false;
        }
    }

    return true;
}

uint32 Vk_RenderWindow::GetVulkanSwapChainNumImages(
    const vk::SurfaceCapabilitiesKHR& surface_capabilities) {
    // Set of images defined in a swap chain may not always be available for
    // application to render to:
    // One may be displayed and one may wait in a queue to be presented
    // If application wants to use more images at the same time it must ask for
    // more images
    uint32 image_count = surface_capabilities.minImageCount + 1;
    if (surface_capabilities.maxImageCount > 0 &&
        image_count > surface_capabilities.maxImageCount) {
        image_count = surface_capabilities.maxImageCount;
    }
    return image_count;
}

vk::SurfaceFormatKHR Vk_RenderWindow::GetVulkanSwapChainFormat(
    const std::vector<vk::SurfaceFormatKHR>& surface_formats) {
    // If the list contains only one entry with undefined format
    // it means that there are no preferred surface formats and any can be
    // chosen
    if (surface_formats.size() == 1 &&
        surface_formats[0].format == vk::Format::eUndefined) {
        return {vk::Format::eR8G8B8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear};
    }

    // Check if list contains most widely used R8 G8 B8 A8 format
    // with nonlinear color space
    for (const vk::SurfaceFormatKHR& surface_format : surface_formats) {
        if (surface_format.format == vk::Format::eR8G8B8A8Unorm) {
            return surface_format;
        }
    }

    // Return the first format from the list
    return surface_formats[0];
}

vk::Extent2D Vk_RenderWindow::GetVulkanSwapChainExtent(
    const vk::SurfaceCapabilitiesKHR& surface_capabilities) {
    // Special value of surface extent is width == height == 0xFFFFFFFF
    // If this is so we define the size by ourselves but it must fit within
    // defined confines
    if (surface_capabilities.currentExtent.width == 0xFFFFFFFF) {
        vk::Extent2D swap_chain_extent = {static_cast<uint32>(m_size.x),
                                          static_cast<uint32>(m_size.y)};
        swap_chain_extent.width = math::Clamp(
            swap_chain_extent.width, surface_capabilities.minImageExtent.width,
            surface_capabilities.maxImageExtent.width);
        swap_chain_extent.height =
            math::Clamp(swap_chain_extent.height,
                        surface_capabilities.minImageExtent.height,
                        surface_capabilities.maxImageExtent.height);
        return swap_chain_extent;
    }

    // Most of the cases we define size of the swap_chain images equal to
    // current window's size
    return surface_capabilities.currentExtent;
}

vk::ImageUsageFlags Vk_RenderWindow::GetVulkanSwapChainUsageFlags(
    const vk::SurfaceCapabilitiesKHR& surface_capabilities) {
    // Color attachment flag must always be supported
    // We can define other usage flags but we always need to check if they are
    // supported
    if (surface_capabilities.supportedUsageFlags &
        vk::ImageUsageFlagBits::eTransferDst) {
        return vk::ImageUsageFlagBits::eColorAttachment |
               vk::ImageUsageFlagBits::eTransferDst;
    }
    return vk::ImageUsageFlags(static_cast<vk::ImageUsageFlagBits>(-1));
}

vk::SurfaceTransformFlagBitsKHR Vk_RenderWindow::GetVulkanSwapChainTransform(
    const vk::SurfaceCapabilitiesKHR& surface_capabilities) {
    // Sometimes images must be transformed before they are presented (i.e. due
    // to device's orienation being other than default orientation)
    // If the specified transform is other than current transform, presentation
    // engine will transform image during presentation operation; this operation
    // may hit performance on some platforms
    // Here we don't want any transformations to occur so if the identity
    // transform is supported use it otherwise just use the same transform as
    // current transform
    if (surface_capabilities.supportedTransforms &
        vk::SurfaceTransformFlagBitsKHR::eIdentity) {
        return vk::SurfaceTransformFlagBitsKHR::eIdentity;
    } else {
        return surface_capabilities.currentTransform;
    }
}

vk::PresentModeKHR Vk_RenderWindow::GetVulkanSwapChainPresentMode(
    const std::vector<vk::PresentModeKHR>& present_modes) {
    // MAILBOX is the lowest latency V-Sync enabled mode (something like
    // triple-buffering) so use it if available
    for (const vk::PresentModeKHR& present_mode : present_modes) {
        if (present_mode == vk::PresentModeKHR::eMailbox) {
            return present_mode;
        }
    }
    // FIFO is the only present mode that is required to be supported
    // by the VulkanSDK.
    return vk::PresentModeKHR::eFifo;
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

void Vk_RenderWindow::CleanCommandBuffers() {
    if (m_device) {
        m_device.waitIdle();

        if (m_present_queue_cmd_buffers.size() > 0 &&
            m_present_queue_cmd_buffers[0]) {
            m_device.freeCommandBuffers(
                m_present_queue_cmd_pool,
                static_cast<uint32>(m_present_queue_cmd_buffers.size()),
                m_present_queue_cmd_buffers.data());
            m_present_queue_cmd_buffers.clear();
        }

        if (m_present_queue_cmd_pool) {
            m_device.destroyCommandPool(m_present_queue_cmd_pool, nullptr);
            m_present_queue_cmd_pool = vk::CommandPool();
        }
    }
}

bool Vk_RenderWindow::OnWindowSizeChanged() {
    // Update the base class attributes
    SDL_GetWindowSize(m_window, &m_size.x, &m_size.y);

    CleanCommandBuffers();

    if (!CreateVulkanSwapChain()) {
        return false;
    }
    if (!CreateVulkanCommandBuffers()) {
        return false;
    }
    return true;
}

}  // namespace engine
