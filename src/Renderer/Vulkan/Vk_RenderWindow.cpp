#include <System/LogManager.hpp>
#include <System/StringFormat.hpp>
#include <System/IO.hpp>

#include "Vk_RenderWindow.hpp"
#include "Vk_Shader.hpp"

namespace engine {

namespace {

const String sTag("Vk_RenderWindow");

const size_t sResourceCount(3);

}  // namespace

struct VertexData {
    math::Vector4<float> position;
    math::Vector4<float> color;
};

Vk_RenderWindow::Vk_RenderWindow()
      : m_window(nullptr),
        m_surface(VK_NULL_HANDLE),
        m_graphics_pipeline(VK_NULL_HANDLE),
        m_graphics_queue_cmd_pool(VK_NULL_HANDLE),
        m_render_pass(VK_NULL_HANDLE),
        m_vertex_buffer(),
        m_render_resources(sResourceCount) {}

Vk_RenderWindow::~Vk_RenderWindow() {
    Destroy();
}

bool Vk_RenderWindow::Create(const String& name, const math::ivec2& size) {
    // Create the window
    math::ivec2 initial_pos(SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    uint32 window_flags(SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE |
                        SDL_WINDOW_VULKAN);
    m_window = SDL_CreateWindow(name.GetData(), initial_pos.x, initial_pos.y,
                                size.x, size.y, window_flags);
    if (!m_window) {
        LogError(sTag, "SDL_CreateWindow fail: "_format(SDL_GetError()));
        return false;
    }

    // Update the base class attributes
    SDL_GetWindowSize(m_window, &m_size.x, &m_size.y);
    m_name = name;

    CreateVulkanSurface();
    CreateVulkanQueues();
    CreateVulkanSwapChain();

    CreateVulkanRenderPass();
    CreateVulkanPipeline();
    CreateVulkanVertexBuffer();

    // CreateRenderingResources
    CreateVulkanCommandBuffers();
    CreateVulkanSemaphores();
    CreateVulkanFences();

    return true;
}

void Vk_RenderWindow::Destroy() {
    Vk_Context& context = Vk_Context::GetInstance();
    VkInstance& instance = context.GetVulkanInstance();
    VkDevice& device = context.GetVulkanDevice();

    if (device) {
        vkDeviceWaitIdle(device);

        for (size_t i = 0; i < m_render_resources.size(); i++) {
            if (m_render_resources[i].framebuffer) {
                vkDestroyFramebuffer(device, m_render_resources[i].framebuffer,
                                     nullptr);
            }
            if (m_render_resources[i].command_buffer) {
                vkFreeCommandBuffers(device, m_graphics_queue_cmd_pool, 1,
                                     &m_render_resources[i].command_buffer);
            }
            if (m_render_resources[i].image_available_semaphore) {
                vkDestroySemaphore(
                    device, m_render_resources[i].image_available_semaphore,
                    nullptr);
            }
            if (m_render_resources[i].finished_rendering_semaphore) {
                vkDestroySemaphore(
                    device, m_render_resources[i].finished_rendering_semaphore,
                    nullptr);
            }
            if (m_render_resources[i].fence) {
                vkDestroyFence(device, m_render_resources[i].fence, nullptr);
            }
        }
        m_render_resources.clear();

        if (m_graphics_queue_cmd_pool) {
            vkDestroyCommandPool(device, m_graphics_queue_cmd_pool, nullptr);
            m_graphics_queue_cmd_pool = VK_NULL_HANDLE;
        }

        if (m_vertex_buffer.handle) {
            vkDestroyBuffer(device, m_vertex_buffer.handle, nullptr);
            m_vertex_buffer.handle = VK_NULL_HANDLE;
        }

        if (m_vertex_buffer.memory) {
            vkFreeMemory(device, m_vertex_buffer.memory, nullptr);
            m_vertex_buffer.memory = VK_NULL_HANDLE;
        }

        if (m_graphics_pipeline) {
            vkDestroyPipeline(device, m_graphics_pipeline, nullptr);
            m_graphics_pipeline = VK_NULL_HANDLE;
        }

        if (m_render_pass) {
            vkDestroyRenderPass(device, m_render_pass, nullptr);
            m_render_pass = VK_NULL_HANDLE;
        }

        if (m_swapchain.handle) {
            vkDestroySwapchainKHR(device, m_swapchain.handle, nullptr);
            m_swapchain.handle = VK_NULL_HANDLE;
            for (size_t i = 0; i < m_swapchain.images.size(); i++) {
                if (m_swapchain.images[i].view) {
                    vkDestroyImageView(device, m_swapchain.images[i].view,
                                       nullptr);
                }
            }
            m_swapchain.images.clear();
        }
    }

    m_present_queue.handle = VK_NULL_HANDLE;
    m_graphics_queue.handle = VK_NULL_HANDLE;

    if (instance && m_surface) {
        vkDestroySurfaceKHR(instance, m_surface, nullptr);
        m_surface = VK_NULL_HANDLE;
    }

    if (m_window) {
        SDL_DestroyWindow(m_window);
        m_window = VK_NULL_HANDLE;
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
        OnWindowResized(m_size);
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
        OnWindowResized(m_size);
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
    static size_t resource_index = 0;

    VkResult result = VK_SUCCESS;

    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.GetVulkanDevice();

    RenderingResourcesData& current_rendering_resource =
        m_render_resources[resource_index];
    uint32_t image_index;

    resource_index = (resource_index + 1) % sResourceCount;

    result = vkWaitForFences(device, 1, &current_rendering_resource.fence,
                             VK_FALSE, 1000000000);
    if (result != VK_SUCCESS) {
        LogError(sTag, "Waiting for fence takes too long");
        return;
    }

    vkResetFences(device, 1, &current_rendering_resource.fence);

    result = vkAcquireNextImageKHR(
        device, m_swapchain.handle, UINT64_MAX,
        current_rendering_resource.image_available_semaphore, VK_NULL_HANDLE,
        &image_index);
    switch (result) {
        case VK_SUCCESS:
        case VK_SUBOPTIMAL_KHR:
            break;
        case VK_ERROR_OUT_OF_DATE_KHR:
            OnWindowResized(m_size);
            return;
        default:
            LogError(sTag,
                     "Problem occurred during swap chain image acquisition");
            return;
    }

    if (!PrepareFrame(current_rendering_resource.command_buffer,
                      m_swapchain.images[image_index],
                      current_rendering_resource.framebuffer)) {
        return;
    }

    VkPipelineStageFlags wait_dst_stage_mask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submit_info = {
        VK_STRUCTURE_TYPE_SUBMIT_INFO,  // sType
        nullptr,                        // pNext
        1,                              // waitSemaphoreCount
        &current_rendering_resource
             .image_available_semaphore,             // pWaitSemaphores
        &wait_dst_stage_mask,                        // pWaitDstStageMask;
        1,                                           // commandBufferCount
        &current_rendering_resource.command_buffer,  // pCommandBuffers
        1,                                           // signalSemaphoreCount
        &current_rendering_resource
             .finished_rendering_semaphore  // pSignalSemaphores
    };

    result = vkQueueSubmit(m_graphics_queue.handle, 1, &submit_info,
                           current_rendering_resource.fence);

    if (result != VK_SUCCESS) {
        LogError(sTag, "Error submitting the command buffers");
        return;
    }

    VkPresentInfoKHR present_info = {
        VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,  // sType
        nullptr,                             // pNext
        1,                                   // waitSemaphoreCount
        &current_rendering_resource
             .finished_rendering_semaphore,  // pWaitSemaphores
        1,                                   // swapchainCount
        &m_swapchain.handle,                 // pSwapchains
        &image_index,                        // pImageIndices
        nullptr                              // pResults
    };

    result = vkQueuePresentKHR(m_present_queue.handle, &present_info);

    switch (result) {
        case VK_SUCCESS:
            break;
        case VK_ERROR_OUT_OF_DATE_KHR:
        case VK_SUBOPTIMAL_KHR:
            OnWindowResized(m_size);
            return;
        default:
            LogError(sTag, "Problem occurred during image presentation");
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

bool Vk_RenderWindow::CreateVulkanSurface() {
    SDL_SysWMinfo wminfo;
    SDL_VERSION(&wminfo.version);
    if (!SDL_GetWindowWMInfo(m_window, &wminfo)) {
        LogFatal(sTag, "Error on SDL_GetWindowWMInfo.");
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
        result = vkCreateWin32SurfaceKHR(instance, &create_info, nullptr,
                                         &m_surface);
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
            vkCreateXcbSurfaceKHR(instance, &create_info, nullptr, &m_surface);
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
            vkCreateXlibSurfaceKHR(instance, &create_info, nullptr, &m_surface);
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
                                           &m_surface);
    }
#else
#error "Unsupported Vulkan subsystem"
#endif

    if (result != VK_SUCCESS) {
        LogFatal(sTag, "Error creating VkSurfaceKHR.");
        return false;
    }

    return true;
}

bool Vk_RenderWindow::CreateVulkanQueues() {
    // Check that the device graphics queue family has WSI support
    VkBool32 wsi_support;
    Vk_Context& context = Vk_Context::GetInstance();
    PhysicalDeviceParameters& physical_device = context.GetPhysicalDevice();
    QueueParameters& graphics_queue = context.GetGraphicsQueue();

    vkGetPhysicalDeviceSurfaceSupportKHR(physical_device.handle,
                                         graphics_queue.family_index, m_surface,
                                         &wsi_support);
    if (!wsi_support) {
        LogFatal(sTag,
                 "Physical device {} doesn't include WSI "
                 "support"_format(physical_device.properties.deviceName));
        return false;
    }

    m_graphics_queue = graphics_queue;
    m_present_queue = graphics_queue;  // We assume that the graphics
                                       // queue can also present
    return true;
}

bool Vk_RenderWindow::CreateVulkanSemaphores() {
    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.GetVulkanDevice();

    VkSemaphoreCreateInfo semaphore_create_info = {
        VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,  // sType
        nullptr,                                  // pNext
        0                                         // flags
    };

    for (size_t i = 0; i < m_render_resources.size(); i++) {
        VkResult result1 =
            vkCreateSemaphore(device, &semaphore_create_info, nullptr,
                              &m_render_resources[i].image_available_semaphore);
        VkResult result2 = vkCreateSemaphore(
            device, &semaphore_create_info, nullptr,
            &m_render_resources[i].finished_rendering_semaphore);
        if (result1 != VK_SUCCESS || result2 != VK_SUCCESS) {
            LogError(sTag, "Could not create semaphores");
            return false;
        }
    }

    return true;
}

bool Vk_RenderWindow::CreateVulkanFences() {
    VkResult result = VK_SUCCESS;

    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.GetVulkanDevice();

    VkFenceCreateInfo fence_create_info = {
        VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,  // sType
        nullptr,                              // pNext
        VK_FENCE_CREATE_SIGNALED_BIT          // flags
    };

    for (size_t i = 0; i < m_render_resources.size(); i++) {
        result = vkCreateFence(device, &fence_create_info, nullptr,
                               &m_render_resources[i].fence);
        if (result != VK_SUCCESS) {
            LogError(sTag, "Could not create fence");
            return false;
        }
    }
    return true;
}

bool Vk_RenderWindow::CreateVulkanSwapChain() {
    VkResult result = VK_SUCCESS;

    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.GetVulkanDevice();
    VkPhysicalDevice physical_device = context.GetPhysicalDevice();

    // Wait all the Device Queues to finish
    if (device) {
        vkDeviceWaitIdle(device);
    }

    // Destroy the old ImageViews
    for (size_t i = 0; i < m_swapchain.images.size(); i++) {
        if (m_swapchain.images[i].view) {
            vkDestroyImageView(device, m_swapchain.images[i].view, nullptr);
            m_swapchain.images[i].view = VK_NULL_HANDLE;
        }
    }
    m_swapchain.images.clear();

    // Get the Surface capabilities
    VkSurfaceCapabilitiesKHR surface_capabilities;

    result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        physical_device, m_surface, &surface_capabilities);
    if (result != VK_SUCCESS) {
        LogError(sTag, "Could not check presentation surface capabilities");
        return false;
    }

    // Query all the supported Surface formats
    uint32 formats_count = 0;
    std::vector<VkSurfaceFormatKHR> surface_formats;
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, m_surface,
                                                  &formats_count, nullptr);
    if (formats_count > 0 && result == VK_SUCCESS) {
        surface_formats.resize(formats_count);
        result = vkGetPhysicalDeviceSurfaceFormatsKHR(
            physical_device, m_surface, &formats_count, surface_formats.data());
    }

    // Check that the surface formats where queried successfully
    if (formats_count == 0 || result != VK_SUCCESS) {
        LogError(sTag,
                 "Error occurred during presentation surface formats "
                 "enumeration");
        return false;
    }

    // Query all the supported Surface present modes
    uint32 present_modes_count = 0;
    std::vector<VkPresentModeKHR> present_modes;

    result = vkGetPhysicalDeviceSurfacePresentModesKHR(
        physical_device, m_surface, &present_modes_count, nullptr);
    if (present_modes_count > 0 && result == VK_SUCCESS) {
        present_modes.resize(present_modes_count);
        result = vkGetPhysicalDeviceSurfacePresentModesKHR(
            physical_device, m_surface, &present_modes_count,
            present_modes.data());
    }

    // Check that the surface present modes where queried successfully
    if (present_modes_count == 0 || result != VK_SUCCESS) {
        LogError(sTag,
                 "Error occurred during presentation surface formats "
                 "enumeration");
        return false;
    }

    // Retreive all the Swapchain related information
    uint32 desired_number_of_images =
        GetVulkanSwapChainNumImages(surface_capabilities);
    VkSurfaceFormatKHR desired_format =
        GetVulkanSwapChainFormat(surface_formats);
    VkExtent2D desired_extent = GetVulkanSwapChainExtent(surface_capabilities);
    VkImageUsageFlags desired_usage =
        GetVulkanSwapChainUsageFlags(surface_capabilities);
    VkSurfaceTransformFlagBitsKHR desired_transform =
        GetVulkanSwapChainTransform(surface_capabilities);
    VkPresentModeKHR desired_present_mode =
        GetVulkanSwapChainPresentMode(present_modes);
    VkSwapchainKHR old_swap_chain = m_swapchain.handle;

    if (static_cast<int>(desired_usage) == -1) {
        return false;
    }

    VkCompositeAlphaFlagBitsKHR composite_alpha =
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    if (!(surface_capabilities.supportedCompositeAlpha & composite_alpha)) {
        composite_alpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
    }

    VkSwapchainCreateInfoKHR swap_chain_create_info = {
        VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,  // sType
        nullptr,                                      // pNext
        VkSwapchainCreateFlagsKHR(),                  // flags
        m_surface,                                    // surface
        desired_number_of_images,                     // minImageCount
        desired_format.format,                        // imageFormat
        desired_format.colorSpace,                    // imageColorSpace
        desired_extent,                               // imageExtent
        1,                                            // imageArrayLayers
        desired_usage,                                // imageUsage
        VK_SHARING_MODE_EXCLUSIVE,                    // imageSharingMode
        0,                                            // queueFamilyIndexCount
        nullptr,                                      // pQueueFamilyIndices
        desired_transform,                            // preTransform
        composite_alpha,                              // compositeAlpha
        desired_present_mode,                         // presentMode
        true,                                         // clipped
        old_swap_chain                                // oldSwapchain
    };

    result = vkCreateSwapchainKHR(device, &swap_chain_create_info, nullptr,
                                  &m_swapchain.handle);
    if (result != VK_SUCCESS) {
        LogFatal(sTag, "Could not create swap chain");
        return false;
    }
    if (old_swap_chain) {
        vkDestroySwapchainKHR(device, old_swap_chain, nullptr);
    }

    // Store all the necesary SwapChain parameters

    // Get the SwapChain format
    m_swapchain.format = desired_format.format;

    // Get the SwapChain images
    uint32 image_count = 0;
    std::vector<VkImage> swapchain_images;
    result = vkGetSwapchainImagesKHR(device, m_swapchain.handle, &image_count,
                                     nullptr);

    if (image_count > 0 && result == VK_SUCCESS) {
        swapchain_images.resize(image_count);
        result = vkGetSwapchainImagesKHR(device, m_swapchain.handle,
                                         &image_count, swapchain_images.data());
    }
    // Check that all the images could be queried
    if (image_count == 0 || result != VK_SUCCESS) {
        LogError(sTag, "Could not get the number of swap chain images");
        return false;
    }
    // Store all the Image handles
    m_swapchain.images.resize(image_count);
    for (size_t i = 0; i < swapchain_images.size(); i++) {
        m_swapchain.images[i].handle = swapchain_images[i];
    }
    // Create all the ImageViews
    for (size_t i = 0; i < m_swapchain.images.size(); i++) {
        VkImageViewCreateInfo image_view_create_info = {
            VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,  // sType
            nullptr,                                   // pNext
            VkImageViewCreateFlags(),                  // flags
            m_swapchain.images[i].handle,              // image
            VK_IMAGE_VIEW_TYPE_2D,                     // viewType
            m_swapchain.format,                        // format
            VkComponentMapping{
                // components
                VK_COMPONENT_SWIZZLE_IDENTITY,  // r
                VK_COMPONENT_SWIZZLE_IDENTITY,  // g
                VK_COMPONENT_SWIZZLE_IDENTITY,  // b
                VK_COMPONENT_SWIZZLE_IDENTITY   // a
            },
            VkImageSubresourceRange{
                VK_IMAGE_ASPECT_COLOR_BIT,  // aspectMask
                0,                          // baseMipLevel
                1,                          // levelCount
                0,                          // baseArrayLayer
                1                           // layerCount
            },
        };

        Vk_Context& context = Vk_Context::GetInstance();

        result = vkCreateImageView(context.GetVulkanDevice(),
                                   &image_view_create_info, nullptr,
                                   &m_swapchain.images[i].view);
        if (result != VK_SUCCESS) {
            LogError(sTag, "Could not create image view for framebuffer.");
            return false;
        }
    }

    return true;
}

bool Vk_RenderWindow::CreateVulkanCommandBuffers() {
    VkResult result = VK_SUCCESS;

    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.GetVulkanDevice();

    // Create the pool for the command buffers
    VkCommandPoolCreateInfo cmd_pool_create_info = {
        VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,  // sType
        nullptr,                                     // pNext
        (VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT |
         VK_COMMAND_POOL_CREATE_TRANSIENT_BIT),  // flags
        m_graphics_queue.family_index            // queueFamilyIndex
    };

    result = vkCreateCommandPool(device, &cmd_pool_create_info, nullptr,
                                 &m_graphics_queue_cmd_pool);
    if (result != VK_SUCCESS) {
        LogError(sTag, "Could not create a command pool");
        return false;
    }

    // Allocate space in the pool for each buffer
    for (size_t i = 0; i < m_render_resources.size(); i++) {
        VkCommandBufferAllocateInfo cmd_buffer_allocate_info = {
            VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,  // sType
            nullptr,                                         // pNext
            m_graphics_queue_cmd_pool,                       // commandPool
            VK_COMMAND_BUFFER_LEVEL_PRIMARY,                 // level
            1                                                // bufferCount
        };
        result =
            vkAllocateCommandBuffers(device, &cmd_buffer_allocate_info,
                                     &m_render_resources[i].command_buffer);
        if (result != VK_SUCCESS) {
            LogError(sTag, "Could not allocate command buffers");
            return false;
        }
    }

    return true;
}

bool Vk_RenderWindow::CreateVulkanRenderPass() {
    VkResult result = VK_SUCCESS;

    // Create the attachment descriptions
    VkAttachmentDescription attachment_descriptions[] = {{
        VkAttachmentDescriptionFlags(),    // flags
        m_swapchain.format,                // format
        VK_SAMPLE_COUNT_1_BIT,             // samples
        VK_ATTACHMENT_LOAD_OP_CLEAR,       // loadOp
        VK_ATTACHMENT_STORE_OP_STORE,      // storeOp
        VK_ATTACHMENT_LOAD_OP_DONT_CARE,   // stencilLoadOp
        VK_ATTACHMENT_STORE_OP_DONT_CARE,  // stencilStoreOp
        VK_IMAGE_LAYOUT_UNDEFINED,         // initialLayout
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR    // finalLayout
    }};

    VkAttachmentReference color_attachment_references[] = {{
        0,                                        // attachment
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL  // layout
    }};

    VkSubpassDescription subpass_descriptions[] = {{
        VkSubpassDescriptionFlags(),      // flags
        VK_PIPELINE_BIND_POINT_GRAPHICS,  // pipelineBindPoint
        0,                                // inputAttachmentCount
        nullptr,                          // pInputAttachments
        1,                                // colorAttachmentCount
        color_attachment_references,      // pColorAttachments
        nullptr,                          // pResolveAttachments
        nullptr,                          // pDepthStencilAttachment
        0,                                // preserveAttachmentCount
        nullptr                           // pPreserveAttachments
    }};

    std::vector<VkSubpassDependency> dependencies = {
        {
            VK_SUBPASS_EXTERNAL,                            // srcSubpass
            0,                                              // dstSubpass
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,           // srcStageMask
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,  // dstStageMask
            VK_ACCESS_MEMORY_READ_BIT,                      // srcAccessMask
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,           // dstAccessMask
            VK_DEPENDENCY_BY_REGION_BIT                     // dependencyFlags
        },
        {
            0,                                              // srcSubpass
            VK_SUBPASS_EXTERNAL,                            // dstSubpass
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,  // srcStageMask
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,           // dstStageMask
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,           // srcAccessMask
            VK_ACCESS_MEMORY_READ_BIT,                      // dstAccessMask
            VK_DEPENDENCY_BY_REGION_BIT                     // dependencyFlags
        }};

    VkRenderPassCreateInfo render_pass_create_info = {
        VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,   // sType
        nullptr,                                     // pNext
        VkRenderPassCreateFlags(),                   // flags
        1,                                           // attachmentCount
        attachment_descriptions,                     // pAttachments
        1,                                           // subpassCount
        subpass_descriptions,                        // pSubpasses
        static_cast<uint32_t>(dependencies.size()),  // dependencyCount
        dependencies.data()                          // pDependencies
    };

    // NOTES: Dependencies are important for performance

    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.GetVulkanDevice();
    result = vkCreateRenderPass(device, &render_pass_create_info, nullptr,
                                &m_render_pass);
    if (result != VK_SUCCESS) {
        LogError(sTag, "Could not create render pass.");
        return false;
    }

    return true;
}

bool Vk_RenderWindow::CreateVulkanPipeline() {
    VkResult result = VK_SUCCESS;

    Vk_Shader vertex_shader_module;
    Vk_Shader fragment_shader_module;

    {
        std::vector<byte> vertex_shader_code;
        std::vector<byte> fragment_shader_code;

        io::FileLoader::LoadFile("shaders/spirv/triangle.vert",
                                 &vertex_shader_code);
        io::FileLoader::LoadFile("shaders/spirv/triangle.frag",
                                 &fragment_shader_code);

        vertex_shader_module.LoadFromMemory(vertex_shader_code.data(),
                                            vertex_shader_code.size(),
                                            ShaderType::Vertex);
        fragment_shader_module.LoadFromMemory(fragment_shader_code.data(),
                                              fragment_shader_code.size(),
                                              ShaderType::Fragment);
    }

    if (!vertex_shader_module.GetModule() ||
        !fragment_shader_module.GetModule()) {
        return false;
    }

    std::vector<VkVertexInputBindingDescription> vertex_binding_descriptions = {
        {
            0,                           // binding
            sizeof(VertexData),          // stride
            VK_VERTEX_INPUT_RATE_VERTEX  // inputRate
        }};

    std::vector<VkVertexInputAttributeDescription>
        vertex_attribute_descriptions = {
            {
                0,                                       // location
                vertex_binding_descriptions[0].binding,  // binding
                VK_FORMAT_R32G32B32A32_SFLOAT,           // format
                offsetof(struct VertexData, position)    // offset
            },
            {
                1,                                       // location
                vertex_binding_descriptions[0].binding,  // binding
                VK_FORMAT_R32G32B32A32_SFLOAT,           // format
                offsetof(struct VertexData, color)       // offset
            }};

    VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info = {
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,  // sType
        nullptr,                                                    // pNext
        0,                                                          // flags;
        static_cast<uint32_t>(vertex_binding_descriptions
                                  .size()),  // vertexBindingDescriptionCount
        vertex_binding_descriptions.data(),  // pVertexBindingDescriptions
        static_cast<uint32_t>(vertex_attribute_descriptions
                                  .size()),   // vertexAttributeDescriptionCount
        vertex_attribute_descriptions.data()  // pVertexAttributeDescriptions
    };

    std::vector<VkPipelineShaderStageCreateInfo> shader_stage_create_infos = {
        // Vertex shader
        {
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,  // sType
            nullptr,                                              // pNext
            VkPipelineShaderStageCreateFlags(),                   // flags
            VK_SHADER_STAGE_VERTEX_BIT,                           // stage
            vertex_shader_module.GetModule(),                     // module
            "main",                                               // pName
            nullptr  // pSpecializationInfo
        },
        // Fragment shader
        {
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,  // sType
            nullptr,                                              // pNext
            VkPipelineShaderStageCreateFlags(),                   // flags
            VK_SHADER_STAGE_FRAGMENT_BIT,                         // stage
            fragment_shader_module.GetModule(),                   // module
            "main",                                               // pName
            nullptr  // pSpecializationInfo
        }};

    VkPipelineInputAssemblyStateCreateInfo input_assembly_state_create_info = {
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,  // sType
        nullptr,                                                      // pNext
        VkPipelineInputAssemblyStateCreateFlags(),                    // flags
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,  // topology
        VK_FALSE                               // primitiveRestartEnable
    };

    VkPipelineViewportStateCreateInfo viewport_state_create_info = {
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,  // sType
        nullptr,                                                // pNext
        VkPipelineViewportStateCreateFlags(),                   // flags
        1,                                                      // viewportCount
        nullptr,                                                // pViewports
        1,                                                      // scissorCount
        nullptr                                                 // pScissors
    };

    VkPipelineRasterizationStateCreateInfo rasterization_state_create_info = {
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,  // sType
        nullptr,                                                     // pNext
        VkPipelineRasterizationStateCreateFlags(),                   // flags
        VK_FALSE,                         // depthClampEnable
        VK_FALSE,                         // rasterizerDiscardEnable
        VK_POLYGON_MODE_FILL,             // polygonMode
        VK_CULL_MODE_BACK_BIT,            // cullMode
        VK_FRONT_FACE_COUNTER_CLOCKWISE,  // frontFace
        VK_FALSE,                         // depthBiasEnable
        0.0f,                             // depthBiasConstantFactor
        0.0f,                             // depthBiasClamp
        0.0f,                             // depthBiasSlopeFactor
        1.0f                              // lineWidth
    };

    VkPipelineMultisampleStateCreateInfo multisample_state_create_info = {
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,  // sType
        nullptr,                                                   // pNext
        VkPipelineMultisampleStateCreateFlags(),                   // flags
        VK_SAMPLE_COUNT_1_BIT,  // rasterizationSamples
        VK_FALSE,               // sampleShadingEnable
        1.0f,                   // minSampleShading
        nullptr,                // pSampleMask
        VK_FALSE,               // alphaToCoverageEnable
        VK_FALSE                // alphaToOneEnable
    };

    VkPipelineColorBlendAttachmentState color_blend_attachment_state = {
        VK_FALSE,              // blendEnable
        VK_BLEND_FACTOR_ONE,   // srcColorBlendFactor
        VK_BLEND_FACTOR_ZERO,  // dstColorBlendFactor
        VK_BLEND_OP_ADD,       // colorBlendOp
        VK_BLEND_FACTOR_ONE,   // srcAlphaBlendFactor
        VK_BLEND_FACTOR_ZERO,  // dstAlphaBlendFactor
        VK_BLEND_OP_ADD,       // alphaBlendOp
        (VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
         VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT)  // colorWriteMask
    };

    VkPipelineColorBlendStateCreateInfo color_blend_state_create_info = {
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,  // sType
        nullptr,                                                   // pNext
        VkPipelineColorBlendStateCreateFlags(),                    // flags
        VK_FALSE,                       // logicOpEnable
        VK_LOGIC_OP_COPY,               // logicOp
        1,                              // attachmentCount
        &color_blend_attachment_state,  // pAttachments
        {0.0f, 0.0f, 0.0f, 0.0f}        // blendConstants[4]
    };

    // Define the pipeline dynamic states
    std::vector<VkDynamicState> dynamic_states = {
        VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR,
    };
    VkPipelineDynamicStateCreateInfo dynamic_state_create_info = {
        VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,  // sType
        nullptr,                                               // pNext
        0,                                                     // flags
        static_cast<uint32_t>(dynamic_states.size()),  // dynamicStateCount
        dynamic_states.data()                          // pDynamicStates
    };

    // Create the PipelineLayout
    VkPipelineLayoutCreateInfo layout_create_info = {
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,  // sType
        nullptr,                                        // pNext
        VkPipelineLayoutCreateFlags(),                  // flags
        0,                                              // setLayoutCount
        nullptr,                                        // pSetLayouts
        0,       // pushConstantRangeCount
        nullptr  // pPushConstantRanges
    };

    VkPipelineLayout pipeline_layout;
    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.GetVulkanDevice();

    result = vkCreatePipelineLayout(device, &layout_create_info, nullptr,
                                    &pipeline_layout);
    if (result != VK_SUCCESS) {
        LogError(sTag, "Could not create pipeline layout");
        return false;
    }

    VkGraphicsPipelineCreateInfo pipeline_create_info = {
        VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,          // sType
        nullptr,                                                  // pNext
        VkPipelineCreateFlags(),                                  // flags
        static_cast<uint32_t>(shader_stage_create_infos.size()),  // stageCount
        shader_stage_create_infos.data(),                         // pStages
        &vertex_input_state_create_info,    // pVertexInputState;
        &input_assembly_state_create_info,  // pInputAssemblyState
        nullptr,                            // pTessellationState
        &viewport_state_create_info,        // pViewportState
        &rasterization_state_create_info,   // pRasterizationState
        &multisample_state_create_info,     // pMultisampleState
        nullptr,                            // pDepthStencilState
        &color_blend_state_create_info,     // pColorBlendState
        &dynamic_state_create_info,         // pDynamicState
        pipeline_layout,                    // layout
        m_render_pass,                      // renderPass
        0,                                  // subpass
        VK_NULL_HANDLE,                     // basePipelineHandle
        -1                                  // basePipelineIndex
    };

    result = vkCreateGraphicsPipelines(device, VkPipelineCache(), 1,
                                       &pipeline_create_info, nullptr,
                                       &m_graphics_pipeline);
    if (result != VK_SUCCESS) {
        LogError(sTag, "Could not create graphics pipeline");
        vkDestroyPipelineLayout(device, pipeline_layout, nullptr);
        return false;
    }

    vkDestroyPipelineLayout(device, pipeline_layout, nullptr);
    return true;
}

bool Vk_RenderWindow::CreateVulkanVertexBuffer() {
    VkResult result = VK_SUCCESS;

    VertexData vertex_data[] = {
        {{-0.7f, -0.7f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
        {{-0.7f, 0.7f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
        {{0.7f, -0.7f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},
        {{0.7f, 0.7f, 0.0f, 1.0f}, {1.0f, 1.0f, 0.0f, 1.0f}}};

    m_vertex_buffer.size = sizeof(vertex_data);

    VkBufferCreateInfo buffer_create_info = {
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,  // sType
        nullptr,                               // pNext
        0,                                     // flags
        m_vertex_buffer.size,                  // size
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,     // sage
        VK_SHARING_MODE_EXCLUSIVE,             // sharingMode
        0,                                     // queueFamilyIndexCount
        nullptr                                // pQueueFamilyIndices
    };

    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.GetVulkanDevice();

    result = vkCreateBuffer(device, &buffer_create_info, nullptr,
                            &m_vertex_buffer.handle);
    if (result != VK_SUCCESS) {
        LogError(sTag, "Could not create a vertex buffer");
        return false;
    }

    if (!AllocateVulkanBufferMemory(m_vertex_buffer.handle,
                                    &m_vertex_buffer.memory)) {
        LogError(sTag, "Could not allocate memory for a vertex buffer");
        return false;
    }

    result = vkBindBufferMemory(device, m_vertex_buffer.handle,
                                m_vertex_buffer.memory, 0);
    if (result != VK_SUCCESS) {
        LogError(sTag, "Could not bind memory for a vertex buffer");
        return false;
    }

    void* vertex_buffer_memory_pointer;
    result =
        vkMapMemory(device, m_vertex_buffer.memory, 0, m_vertex_buffer.size, 0,
                    &vertex_buffer_memory_pointer);
    if (result != VK_SUCCESS) {
        LogError(sTag,
                 "Could not map memory and upload data to a vertex buffer");
        return false;
    }

    std::memcpy(vertex_buffer_memory_pointer, vertex_data,
                m_vertex_buffer.size);

    VkMappedMemoryRange flush_range = {
        VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,  // sType
        nullptr,                                // pNext
        m_vertex_buffer.memory,                 // memory
        0,                                      // offset
        VK_WHOLE_SIZE                           // size
    };
    vkFlushMappedMemoryRanges(device, 1, &flush_range);

    vkUnmapMemory(device, m_vertex_buffer.memory);

    return true;
}

bool Vk_RenderWindow::CreateVulkanFrameBuffer(VkFramebuffer& framebuffer,
                                              VkImageView& image_view) {
    VkResult result = VK_SUCCESS;

    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.GetVulkanDevice();

    if (framebuffer) {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }

    VkFramebufferCreateInfo framebuffer_create_info = {
        VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,  // sType
        nullptr,                                    // pNext
        VkFramebufferCreateFlags(),                 // flags
        m_render_pass,                              // renderPass
        1,                                          // attachmentCount
        &image_view,                                // pAttachments
        static_cast<uint32_t>(m_size.x),            // width
        static_cast<uint32_t>(m_size.y),            // height
        1                                           // layers
    };

    result = vkCreateFramebuffer(device, &framebuffer_create_info, nullptr,
                                 &framebuffer);
    if (result != VK_SUCCESS) {
        LogError(sTag, "Could not create a framebuffer.");
        return false;
    }

    return true;
}

bool Vk_RenderWindow::PrepareFrame(VkCommandBuffer command_buffer,
                                   ImageParameters& image_parameters,
                                   VkFramebuffer& framebuffer) {
    VkResult result = VK_SUCCESS;

    if (!CreateVulkanFrameBuffer(framebuffer, image_parameters.view)) {
        return false;
    }

    VkCommandBufferBeginInfo command_buffer_begin_info = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,  // sType
        nullptr,                                      // pNext
        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,  // flags
        nullptr                                       // pInheritanceInfo
    };

    vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info);

    VkImageSubresourceRange image_subresource_range = {
        VK_IMAGE_ASPECT_COLOR_BIT,  // aspectMask
        0,                          // baseMipLevel
        1,                          // levelCount
        0,                          // baseArrayLayer
        1                           // layerCount
    };

    if (m_present_queue.handle != m_graphics_queue.handle) {
        VkImageMemoryBarrier barrier_from_present_to_draw = {
            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,  // sType
            nullptr,                                 // pNext
            VK_ACCESS_MEMORY_READ_BIT,               // srcAccessMask
            VK_ACCESS_MEMORY_READ_BIT,               // dstAccessMask
            VK_IMAGE_LAYOUT_UNDEFINED,               // oldLayout
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,         // newLayout
            m_present_queue.family_index,            // srcQueueFamilyIndex
            m_graphics_queue.family_index,           // dstQueueFamilyIndex
            image_parameters.handle,                 // image
            image_subresource_range                  // subresourceRange
        };
        vkCmdPipelineBarrier(
            command_buffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0,
            nullptr, 1, &barrier_from_present_to_draw);
    }

    VkClearValue clear_value = {
        {1.0f, 0.8f, 0.4f, 0.0f},  // color
    };

    VkRenderPassBeginInfo render_pass_begin_info = {
        VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,  // sType
        nullptr,                                   // pNext
        m_render_pass,                             // renderPass
        framebuffer,                               // framebuffer
        {
            // renderArea
            {
                // offset
                0,  // x
                0   // y
            },
            {
                // extent
                static_cast<uint32_t>(m_size.x),  // width
                static_cast<uint32_t>(m_size.y),  // height
            },
        },
        1,            // clearValueCount
        &clear_value  // pClearValues
    };

    vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info,
                         VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      m_graphics_pipeline);

    VkViewport viewport = {
        0.0f,                          // x
        0.0f,                          // y
        static_cast<float>(m_size.x),  // width
        static_cast<float>(m_size.y),  // height
        0.0f,                          // minDepth
        1.0f                           // maxDepth
    };

    VkRect2D scissor = {{
                            // offset
                            0,  // x
                            0   // y
                        },
                        {
                            // extent
                            static_cast<uint32_t>(m_size.x),  // width
                            static_cast<uint32_t>(m_size.y),  // height
                        }};

    vkCmdSetViewport(command_buffer, 0, 1, &viewport);
    vkCmdSetScissor(command_buffer, 0, 1, &scissor);

    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(command_buffer, 0, 1, &m_vertex_buffer.handle,
                           &offset);

    vkCmdDraw(command_buffer, 4, 1, 0, 0);

    vkCmdEndRenderPass(command_buffer);

    if (m_graphics_queue.handle != m_present_queue.handle) {
        VkImageMemoryBarrier barrier_from_draw_to_present = {
            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,  // sType
            nullptr,                                 // pNext
            VK_ACCESS_MEMORY_READ_BIT,               // srcAccessMask
            VK_ACCESS_MEMORY_READ_BIT,               // dstAccessMask
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,         // oldLayout
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,         // newLayout
            m_graphics_queue.family_index,           // srcQueueFamilyIndex
            m_present_queue.family_index,            // dstQueueFamilyIndex
            image_parameters.handle,                 // image
            image_subresource_range                  // subresourceRange
        };
        vkCmdPipelineBarrier(
            command_buffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1,
            &barrier_from_draw_to_present);
    }

    result = vkEndCommandBuffer(command_buffer);
    if (result != VK_SUCCESS) {
        LogError(sTag, "Could not record command buffer");
        return false;
    }

    return true;
}

uint32 Vk_RenderWindow::GetVulkanSwapChainNumImages(
    const VkSurfaceCapabilitiesKHR& surface_capabilities) {
    // Set of images defined in a swap chain may not always be available for
    // application to render to:
    // One may be displayed and one may wait in a queue to be presented
    // If application wants to use more images at the same time it must ask
    // for more images
    uint32 image_count = surface_capabilities.minImageCount + 1;
    if (surface_capabilities.maxImageCount > 0 &&
        image_count > surface_capabilities.maxImageCount) {
        image_count = surface_capabilities.maxImageCount;
    }
    return image_count;
}

VkSurfaceFormatKHR Vk_RenderWindow::GetVulkanSwapChainFormat(
    const std::vector<VkSurfaceFormatKHR>& surface_formats) {
    // If the list contains only one entry with undefined format
    // it means that there are no preferred surface formats and any can be
    // chosen
    if (surface_formats.size() == 1 &&
        surface_formats[0].format == VK_FORMAT_UNDEFINED) {
        return {VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
    }

    // Check if list contains most widely used R8 G8 B8 A8 format
    // with nonlinear color space
    for (const VkSurfaceFormatKHR& surface_format : surface_formats) {
        if (surface_format.format == VK_FORMAT_R8G8B8A8_UNORM) {
            return surface_format;
        }
    }

    // Return the first format from the list
    return surface_formats[0];
}

VkExtent2D Vk_RenderWindow::GetVulkanSwapChainExtent(
    const VkSurfaceCapabilitiesKHR& surface_capabilities) {
    // Special value of surface extent is width == height == 0xFFFFFFFF
    // If this is so we define the size by ourselves but it must fit within
    // defined confines
    if (surface_capabilities.currentExtent.width == 0xFFFFFFFF) {
        VkExtent2D swap_chain_extent = {static_cast<uint32>(m_size.x),
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

VkImageUsageFlags Vk_RenderWindow::GetVulkanSwapChainUsageFlags(
    const VkSurfaceCapabilitiesKHR& surface_capabilities) {
    // Color attachment flag must always be supported
    // We can define other usage flags but we always need to check if they
    // are supported
    if (surface_capabilities.supportedUsageFlags &
        VK_IMAGE_USAGE_TRANSFER_DST_BIT) {
        return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
               VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }
    return VkImageUsageFlags(static_cast<VkImageUsageFlagBits>(-1));
}

VkSurfaceTransformFlagBitsKHR Vk_RenderWindow::GetVulkanSwapChainTransform(
    const VkSurfaceCapabilitiesKHR& surface_capabilities) {
    // Sometimes images must be transformed before they are presented (i.e.
    // due to device's orienation being other than default orientation)
    // If the specified transform is other than current transform,
    // presentation engine will transform image during presentation operation;
    // this operation
    // may hit performance on some platforms
    // Here we don't want any transformations to occur so if the identity
    // transform is supported use it otherwise just use the same transform
    // as current transform
    if (surface_capabilities.supportedTransforms &
        VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
        return VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    } else {
        return surface_capabilities.currentTransform;
    }
}

VkPresentModeKHR Vk_RenderWindow::GetVulkanSwapChainPresentMode(
    const std::vector<VkPresentModeKHR>& present_modes) {
    // MAILBOX is the lowest latency V-Sync enabled mode (something like
    // triple-buffering) so use it if available
    for (const VkPresentModeKHR& present_mode : present_modes) {
        if (present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return present_mode;
        }
    }
    // FIFO is the only present mode that is required to be supported
    // by the VulkanSDK.
    return VK_PRESENT_MODE_FIFO_KHR;
}

bool Vk_RenderWindow::AllocateVulkanBufferMemory(VkBuffer buffer,
                                                 VkDeviceMemory* memory) {
    VkResult result = VK_SUCCESS;

    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.GetVulkanDevice();
    VkPhysicalDevice physical_device = context.GetPhysicalDevice();

    VkMemoryRequirements buffer_memory_requirements;
    vkGetBufferMemoryRequirements(device, buffer, &buffer_memory_requirements);

    VkPhysicalDeviceMemoryProperties memory_properties;
    vkGetPhysicalDeviceMemoryProperties(physical_device, &memory_properties);

    for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++) {
        if ((buffer_memory_requirements.memoryTypeBits & (1 << i)) &&
            (memory_properties.memoryTypes[i].propertyFlags &
             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
            VkMemoryAllocateInfo memory_allocate_info = {
                VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,  // sType
                nullptr,                                 // pNext
                buffer_memory_requirements.size,         // allocationSize
                i                                        // memoryTypeIndex
            };
            result = vkAllocateMemory(device, &memory_allocate_info, nullptr,
                                      memory);
            if (result == VK_SUCCESS) {
                return true;
            }
        }
    }

    return false;
}

void Vk_RenderWindow::OnWindowResized(const math::ivec2& size) {
    // Update the base class attributes
    m_size = size;

    // Recreate the Vulkan Swapchain
    CreateVulkanSwapChain();
}

void Vk_RenderWindow::OnAppWillEnterBackground() {

}

void Vk_RenderWindow::OnAppDidEnterBackground() {

}

void Vk_RenderWindow::OnAppWillEnterForeground() {

}

void Vk_RenderWindow::OnAppDidEnterForeground() {

}

}  // namespace engine
