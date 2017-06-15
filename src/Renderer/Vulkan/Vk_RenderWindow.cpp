#include <System/LogManager.hpp>
#include <System/StringFormat.hpp>
#include <System/IO.hpp>

#include "Vk_RenderWindow.hpp"
#include "Vk_Shader.hpp"

namespace engine {

Vk_RenderWindow::Vk_RenderWindow() : m_window(nullptr) {}

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

    CreateVulkanSurface();
    CreateVulkanQueues();
    CreateVulkanSemaphores();
    CreateVulkanSwapChain();

    CreateVulkanRenderPass();
    CreateVulkanFrameBuffers();
    CreateVulkanPipeline();

    CreateVulkanCommandBuffers();

    return true;
}

void Vk_RenderWindow::Destroy() {
    ClearPipeline();

    Vk_Context& context = Vk_Context::GetInstance();
    vk::Instance& instance = context.GetVulkanInstance();
    vk::Device& device = context.GetVulkanDevice();

    if (device) {
        device.waitIdle();

        if (m_image_avaliable_semaphore) {
            device.destroySemaphore(m_image_avaliable_semaphore, nullptr);
            m_image_avaliable_semaphore = nullptr;
        }
        if (m_rendering_finished_semaphore) {
            device.destroySemaphore(m_rendering_finished_semaphore, nullptr);
            m_rendering_finished_semaphore = nullptr;
        }
        if (m_swapchain.handle) {
            device.destroySwapchainKHR(m_swapchain.handle, nullptr);
            m_swapchain.handle = nullptr;
            for (size_t i = 0; i < m_swapchain.images.size(); i++) {
                if (m_swapchain.images[i].view) {
                    device.destroyImageView(m_swapchain.images[i].view,
                                            nullptr);
                    m_swapchain.images[i].view = nullptr;
                }
            }
            m_swapchain.images.clear();
        }
    }

    m_present_queue.handle = nullptr;
    m_graphics_queue.handle = nullptr;

    if (instance && m_surface) {
        instance.destroySurfaceKHR(m_surface, nullptr);
        m_surface = nullptr;
    }

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
    Vk_Context& context = Vk_Context::GetInstance();
    result = context.GetVulkanDevice().acquireNextImageKHR(
        m_swapchain.handle, UINT64_MAX, m_image_avaliable_semaphore,
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
        1,                                           // waitSemaphoreCount
        &m_image_avaliable_semaphore,                // pWaitSemaphores
        &wait_dst_stage_mask,                        // pWaitDstStageMask;
        1,                                           // commandBufferCount
        &m_graphics_queue_cmd_buffers[image_index],  // pCommandBuffers
        1,                                           // signalSemaphoreCount
        &m_rendering_finished_semaphore              // pSignalSemaphores
    };

    result = m_present_queue.handle.submit(1, &submit_info, vk::Fence());
    if (result != vk::Result::eSuccess) {
        LogError("Vk_RenderWindow", "Error submitting the command buffers");
        return;
    }

    vk::PresentInfoKHR present_info{
        1,                                // waitSemaphoreCount
        &m_rendering_finished_semaphore,  // pWaitSemaphores
        1,                                // swapchainCount
        &m_swapchain.handle,              // pSwapchains
        &image_index,                     // pImageIndices
        nullptr                           // pResults
    };

    result = m_present_queue.handle.presentKHR(&present_info);
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

bool Vk_RenderWindow::CreateVulkanSurface() {
    SDL_SysWMinfo wminfo;
    SDL_VERSION(&wminfo.version);
    if (!SDL_GetWindowWMInfo(m_window, &wminfo)) return false;

    vk::Result result;

    Vk_Context& context = Vk_Context::GetInstance();
    vk::Instance& instance = context.GetVulkanInstance();

#if defined(VK_USE_PLATFORM_WIN32_KHR)
    if (wminfo.subsystem == SDL_SYSWM_WINDOWS) {
        vk::Win32SurfaceCreateInfoKHR create_info{
            vk::Win32SurfaceCreateFlagsKHR(),  // flags
            GetModuleHandle(nullptr),          // hinstance
            wminfo.info.win.window             // hwnd
        };
        result =
            instance.createWin32SurfaceKHR(&create_info, nullptr, &m_surface);
    }
#elif defined(VK_USE_PLATFORM_XCB_KHR)
    if (wminfo.subsystem == SDL_SYSWM_X11) {
        vk::XcbSurfaceCreateInfoKHR create_info{
            vk::XcbSurfaceCreateFlagsKHR(),                    // flags
            XGetXCBConnection(wminfo.info.x11.display),        // connection
            static_cast<xcb_window_t>(wminfo.info.x11.window)  // window
        };
        result =
            instance.createXcbSurfaceKHR(&create_info, nullptr, &m_surface);
    }
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
    if (wminfo.subsystem == SDL_SYSWM_X11) {
        vk::XlibSurfaceCreateInfoKHR create_info{
            vk::XlibSurfaceCreateFlagsKHR(),  // flags
            wminfo.info.x11.display,          // dpy
            wminfo.info.x11.window            // window
        };
        result =
            instance.createXlibSurfaceKHR(&create_info, nullptr, &m_surface);
    }
#endif
    else {
        LogFatal("Vk_RenderWindow", "Unsupported subsystem");
        return false;
    }

    if (result != vk::Result::eSuccess) {
        LogFatal("Vk_RenderWindow", "Error creating VkSurfaceKHR.");
        return false;
    }

    return true;
}

bool Vk_RenderWindow::CreateVulkanQueues() {
    // Check that the device graphics queue family has WSI support
    vk::Bool32 wsi_support;
    Vk_Context& context = Vk_Context::GetInstance();
    PhysicalDeviceParameters& physical_device = context.GetPhysicalDevice();
    QueueParameters graphics_queue = context.GetGraphicsQueue();
    physical_device.handle.getSurfaceSupportKHR(graphics_queue.index, m_surface,
                                                &wsi_support);
    if (!wsi_support) {
        LogFatal("Vk_RenderWindow",
                 "Physical device {} doesn't include WSI "
                 "support"_format(physical_device.properties.deviceName));
        return false;
    }

    m_graphics_queue = graphics_queue;
    m_present_queue = graphics_queue;
    return true;
}

bool Vk_RenderWindow::CreateVulkanSemaphores() {
    Vk_Context& context = Vk_Context::GetInstance();
    vk::Device& device = context.GetVulkanDevice();
    vk::SemaphoreCreateInfo info{vk::SemaphoreCreateFlags()};
    vk::Result result1 =
        device.createSemaphore(&info, nullptr, &m_image_avaliable_semaphore);
    vk::Result result2 =
        device.createSemaphore(&info, nullptr, &m_rendering_finished_semaphore);
    if (result1 != vk::Result::eSuccess || result2 != vk::Result::eSuccess) {
        LogError("Vk_RenderWindow", "Could not create semaphores");
        return false;
    }
    return true;
}

bool Vk_RenderWindow::CreateVulkanSwapChain() {
    vk::Result result;

    Vk_Context& context = Vk_Context::GetInstance();
    vk::Device& device = context.GetVulkanDevice();
    vk::PhysicalDevice physical_device = context.GetPhysicalDevice();

    // Wait all the Device Queues to finish
    if (device) {
        device.waitIdle();
    }

    // Destroy the old ImageViews
    for (size_t i = 0; i < m_swapchain.images.size(); i++) {
        if (m_swapchain.images[i].view) {
            device.destroyImageView(m_swapchain.images[i].view, nullptr);
            m_swapchain.images[i].view = nullptr;
        }
    }
    m_swapchain.images.clear();

    // Get the Surface capabilities
    vk::SurfaceCapabilitiesKHR surface_capabilities;
    result = physical_device.getSurfaceCapabilitiesKHR(m_surface,
                                                       &surface_capabilities);
    if (result != vk::Result::eSuccess) {
        LogError("Vk_RenderWindow",
                 "Could not check presentation surface capabilities");
        return false;
    }

    // Query all the supported Surface formats
    uint32 formats_count = 0;
    std::vector<vk::SurfaceFormatKHR> surface_formats;
    result = physical_device.getSurfaceFormatsKHR(m_surface, &formats_count,
                                                  nullptr);
    if (formats_count > 0 && result == vk::Result::eSuccess) {
        surface_formats.resize(formats_count);
        result = physical_device.getSurfaceFormatsKHR(m_surface, &formats_count,
                                                      surface_formats.data());
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
    result = physical_device.getSurfacePresentModesKHR(
        m_surface, &present_modes_count, nullptr);
    if (present_modes_count > 0 && result == vk::Result::eSuccess) {
        present_modes.resize(present_modes_count);
        result = physical_device.getSurfacePresentModesKHR(
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
    vk::SwapchainKHR old_swap_chain = m_swapchain.handle;

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

    result = device.createSwapchainKHR(&swap_chain_create_info, nullptr,
                                       &m_swapchain.handle);
    if (result != vk::Result::eSuccess) {
        LogFatal("Vk_RenderWindow", "Could not create swap chain");
        return false;
    }
    if (old_swap_chain) {
        device.destroySwapchainKHR(old_swap_chain, nullptr);
    }

    // Store all the necesary SwapChain parameters

    // Get the SwapChain format
    m_swapchain.format = desired_format.format;

    // Get the SwapChain images
    uint32 image_count = 0;
    std::vector<vk::Image> swapchain_images;
    result =
        device.getSwapchainImagesKHR(m_swapchain.handle, &image_count, nullptr);

    if (image_count > 0 && result == vk::Result::eSuccess) {
        swapchain_images.resize(image_count);
        result = device.getSwapchainImagesKHR(m_swapchain.handle, &image_count,
                                              swapchain_images.data());
    }
    // Check that all the images could be queried
    if (image_count == 0 || result != vk::Result::eSuccess) {
        LogError("Vk_RenderWindow",
                 "Could not get the number of swap chain images");
        return false;
    }
    // Store all the Image handles
    m_swapchain.images.resize(image_count);
    for (size_t i = 0; i < swapchain_images.size(); i++) {
        m_swapchain.images[i].handle = swapchain_images[i];
    }
    // Create all the ImageViews
    for (size_t i = 0; i < m_swapchain.images.size(); i++) {
        vk::ImageViewCreateInfo image_view_create_info{
            vk::ImageViewCreateFlags(),    // flags
            m_swapchain.images[i].handle,  // image
            vk::ImageViewType::e2D,        // viewType
            m_swapchain.format,            // format
            vk::ComponentMapping{
                // components
                vk::ComponentSwizzle::eIdentity,  // r
                vk::ComponentSwizzle::eIdentity,  // g
                vk::ComponentSwizzle::eIdentity,  // b
                vk::ComponentSwizzle::eIdentity   // a
            },
            vk::ImageSubresourceRange{
                vk::ImageAspectFlagBits::eColor,  // aspectMask
                0,                                // baseMipLevel
                1,                                // levelCount
                0,                                // baseArrayLayer
                1                                 // layerCount
            },
        };

        Vk_Context& context = Vk_Context::GetInstance();
        result = context.GetVulkanDevice().createImageView(
            &image_view_create_info, nullptr, &m_swapchain.images[i].view);
        if (result != vk::Result::eSuccess) {
            LogError("Vk_RenderWindow",
                     "Could not create image view for framebuffer.");
            return false;
        }
    }

    return true;
}

bool Vk_RenderWindow::CreateVulkanCommandBuffers() {
    vk::Result result;

    Vk_Context& context = Vk_Context::GetInstance();
    vk::Device& device = context.GetVulkanDevice();

    // Create the pool for the command buffers
    vk::CommandPoolCreateInfo cmd_pool_create_info = {
        vk::CommandPoolCreateFlags(),  // flags
        m_present_queue.index          // queueFamilyIndex
    };
    result = device.createCommandPool(&cmd_pool_create_info, nullptr,
                                      &m_graphics_queue_cmd_pool);
    if (result != vk::Result::eSuccess) {
        LogError("Vk_RenderWindow", "Could not create a command pool");
        return false;
    }

    // Reserve a command buffer for each image
    m_graphics_queue_cmd_buffers.resize(m_swapchain.images.size());

    // Allocate space in the pool for each buffer
    vk::CommandBufferAllocateInfo cmd_buffer_allocate_info{
        m_graphics_queue_cmd_pool,                      // commandPool
        vk::CommandBufferLevel::ePrimary,               // level
        static_cast<uint32>(m_swapchain.images.size())  // bufferCount
    };
    result = device.allocateCommandBuffers(&cmd_buffer_allocate_info,
                                           m_graphics_queue_cmd_buffers.data());
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

bool Vk_RenderWindow::CreateVulkanRenderPass() {
    vk::Result result;

    // Create the attachment descriptions
    vk::AttachmentDescription attachment_descriptions[] = {{
        vk::AttachmentDescriptionFlags(),  // flags
        m_swapchain.format,                // format
        vk::SampleCountFlagBits::e1,       // samples
        vk::AttachmentLoadOp::eClear,      // loadOp
        vk::AttachmentStoreOp::eStore,     // storeOp
        vk::AttachmentLoadOp::eDontCare,   // stencilLoadOp
        vk::AttachmentStoreOp::eDontCare,  // stencilStoreOp
        vk::ImageLayout::ePresentSrcKHR,   // initialLayout;
        vk::ImageLayout::ePresentSrcKHR    // finalLayout
    }};

    vk::AttachmentReference color_attachment_references[] = {{
        0,                                        // attachment
        vk::ImageLayout::eColorAttachmentOptimal  // layout
    }};

    vk::SubpassDescription subpass_descriptions[] = {{
        vk::SubpassDescriptionFlags(),     // flags
        vk::PipelineBindPoint::eGraphics,  // pipelineBindPoint
        0,                                 // inputAttachmentCount
        nullptr,                           // pInputAttachments
        1,                                 // colorAttachmentCount
        color_attachment_references,       // pColorAttachments
        nullptr,                           // pResolveAttachments
        nullptr,                           // pDepthStencilAttachment
        0,                                 // preserveAttachmentCount
        nullptr                            // pPreserveAttachments
    }};

    vk::RenderPassCreateInfo render_pass_create_info = {
        vk::RenderPassCreateFlags(),  // flags
        1,                            // attachmentCount
        attachment_descriptions,      // pAttachments
        1,                            // subpassCount
        subpass_descriptions,         // pSubpasses
        0,                            // dependencyCount
        nullptr                       // pDependencies
    };

    // NOTES: Dependencies are important for performance

    Vk_Context& context = Vk_Context::GetInstance();
    result = context.GetVulkanDevice().createRenderPass(
        &render_pass_create_info, nullptr, &m_render_pass);
    if (result != vk::Result::eSuccess) {
        LogError("Vk_RenderWindow", "Could not create render pass.");
        return false;
    }

    return true;
}

bool Vk_RenderWindow::CreateVulkanFrameBuffers() {
    vk::Result result;

    // Create the FrameBuffers
    m_framebuffers.resize(m_swapchain.images.size());
    for (size_t i = 0; i < m_framebuffers.size(); i++) {
        vk::FramebufferCreateInfo framebuffer_create_info{
            vk::FramebufferCreateFlags(),  // flags
            m_render_pass,                 // renderPass
            1,                             // attachmentCount
            &m_swapchain.images[i].view,   // pAttachments
            300,                           // width
            300,                           // height
            1                              // layers
        };

        Vk_Context& context = Vk_Context::GetInstance();
        result = context.GetVulkanDevice().createFramebuffer(
            &framebuffer_create_info, nullptr, &m_framebuffers[i]);
        if (result != vk::Result::eSuccess) {
            LogError("Vk_RenderWindow", "Could not create a framebuffer.");
            return false;
        }
    }

    return true;
}

bool Vk_RenderWindow::CreateVulkanPipeline() {
    vk::Result result;

    Vk_Shader vertex_shader_module;
    Vk_Shader fragment_shader_module;

    {
        std::vector<byte> vertex_shader_code;
        std::vector<byte> fragment_shader_code;

        io::FileLoader::LoadFile("data/shaders/spirv/triangle.vert",
                                 &vertex_shader_code);
        io::FileLoader::LoadFile("data/shaders/spirv/triangle.frag",
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

    std::vector<vk::PipelineShaderStageCreateInfo> shader_stage_create_infos = {
        // Vertex shader
        {
            vk::PipelineShaderStageCreateFlags(),  // flags
            vk::ShaderStageFlagBits::eVertex,      // stage
            vertex_shader_module.GetModule(),      // module
            "main",                                // pName
            nullptr                                // pSpecializationInfo
        },
        // Fragment shader
        {
            vk::PipelineShaderStageCreateFlags(),  // flags
            vk::ShaderStageFlagBits::eFragment,    // stage
            fragment_shader_module.GetModule(),    // module
            "main",                                // pName
            nullptr                                // pSpecializationInfo
        }};

    vk::PipelineVertexInputStateCreateInfo vertex_input_state_create_info = {
        vk::PipelineVertexInputStateCreateFlags(),  // flags;
        0,        // vertexBindingDescriptionCount
        nullptr,  // pVertexBindingDescriptions
        0,        // vertexAttributeDescriptionCount
        nullptr   // pVertexAttributeDescriptions
    };

    vk::PipelineInputAssemblyStateCreateInfo input_assembly_state_create_info =
        {
            vk::PipelineInputAssemblyStateCreateFlags(),  // flags
            vk::PrimitiveTopology::eTriangleList,         // topology
            VK_FALSE  // primitiveRestartEnable
        };

    vk::Viewport viewport = {
        0.0f,    // x
        0.0f,    // y
        300.0f,  // width
        300.0f,  // height
        0.0f,    // minDepth
        1.0f     // maxDepth
    };

    vk::Rect2D scissor = {{
                              // offset
                              0,  // x
                              0   // y
                          },
                          {
                              // extent
                              300,  // width
                              300   // height
                          }};

    vk::PipelineViewportStateCreateInfo viewport_state_create_info = {
        vk::PipelineViewportStateCreateFlags(),  // flags
        1,                                       // viewportCount
        &viewport,                               // pViewports
        1,                                       // scissorCount
        &scissor                                 // pScissors
    };

    vk::PipelineRasterizationStateCreateInfo rasterization_state_create_info = {
        vk::PipelineRasterizationStateCreateFlags(),  // flags
        VK_FALSE,                                     // depthClampEnable
        VK_FALSE,                                     // rasterizerDiscardEnable
        vk::PolygonMode::eFill,                       // polygonMode
        vk::CullModeFlagBits::eBack,                  // cullMode
        vk::FrontFace::eCounterClockwise,             // frontFace
        VK_FALSE,                                     // depthBiasEnable
        0.0f,                                         // depthBiasConstantFactor
        0.0f,                                         // depthBiasClamp
        0.0f,                                         // depthBiasSlopeFactor
        1.0f                                          // lineWidth
    };

    vk::PipelineMultisampleStateCreateInfo multisample_state_create_info = {
        vk::PipelineMultisampleStateCreateFlags(),  // flags
        vk::SampleCountFlagBits::e1,                // rasterizationSamples
        VK_FALSE,                                   // sampleShadingEnable
        1.0f,                                       // minSampleShading
        nullptr,                                    // pSampleMask
        VK_FALSE,                                   // alphaToCoverageEnable
        VK_FALSE                                    // alphaToOneEnable
    };

    vk::PipelineColorBlendAttachmentState color_blend_attachment_state = {
        VK_FALSE,                // blendEnable
        vk::BlendFactor::eOne,   // srcColorBlendFactor
        vk::BlendFactor::eZero,  // dstColorBlendFactor
        vk::BlendOp::eAdd,       // colorBlendOp
        vk::BlendFactor::eOne,   // srcAlphaBlendFactor
        vk::BlendFactor::eZero,  // dstAlphaBlendFactor
        vk::BlendOp::eAdd,       // alphaBlendOp
        vk::ColorComponentFlagBits::eR |
            vk::ColorComponentFlagBits::eG |  // colorWriteMask
            vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA};

    vk::PipelineColorBlendStateCreateInfo color_blend_state_create_info = {
        vk::PipelineColorBlendStateCreateFlags(),  // flags
        VK_FALSE,                                  // logicOpEnable
        vk::LogicOp::eCopy,                        // logicOp
        1,                                         // attachmentCount
        &color_blend_attachment_state,             // pAttachments
        {0.0f, 0.0f, 0.0f, 0.0f}                   // blendConstants[4]
    };

    // Create the PipelineLayout
    vk::PipelineLayoutCreateInfo layout_create_info = {
        vk::PipelineLayoutCreateFlags(),  // flags
        0,                                // setLayoutCount
        nullptr,                          // pSetLayouts
        0,                                // pushConstantRangeCount
        nullptr                           // pPushConstantRanges
    };

    vk::PipelineLayout pipeline_layout;
    Vk_Context& context = Vk_Context::GetInstance();
    vk::Device& device = context.GetVulkanDevice();
    if (device.createPipelineLayout(&layout_create_info, nullptr,
                                    &pipeline_layout) != vk::Result::eSuccess) {
        LogError("Vk_RenderWindow", "Could not create pipeline layout");
        return false;
    }

    vk::GraphicsPipelineCreateInfo pipeline_create_info = {
        vk::PipelineCreateFlags(),                                // flags
        static_cast<uint32_t>(shader_stage_create_infos.size()),  // stageCount
        &shader_stage_create_infos[0],                            // pStages
        &vertex_input_state_create_info,    // pVertexInputState;
        &input_assembly_state_create_info,  // pInputAssemblyState
        nullptr,                            // pTessellationState
        &viewport_state_create_info,        // pViewportState
        &rasterization_state_create_info,   // pRasterizationState
        &multisample_state_create_info,     // pMultisampleState
        nullptr,                            // pDepthStencilState
        &color_blend_state_create_info,     // pColorBlendState
        nullptr,                            // pDynamicState
        pipeline_layout,                    // layout
        m_render_pass,                      // renderPass
        0,                                  // subpass
        vk::Pipeline(),                     // basePipelineHandle
        -1                                  // basePipelineIndex
    };

    result = device.createGraphicsPipelines(vk::PipelineCache(), 1,
                                            &pipeline_create_info, nullptr,
                                            &m_graphics_pipeline);
    if (result != vk::Result::eSuccess) {
        LogError("Vk_RenderWindow", "Could not create graphics pipeline");
        device.destroyPipelineLayout(pipeline_layout, nullptr);
        return false;
    }

    device.destroyPipelineLayout(pipeline_layout, nullptr);
    return true;
}

bool Vk_RenderWindow::RecordCommandBuffers() {
    vk::Result result;

    vk::CommandBufferBeginInfo cmd_buffer_begin_info{
        vk::CommandBufferUsageFlagBits::eSimultaneousUse,  // flags
        nullptr                                            // pInheritanceInfo
    };

    vk::ClearColorValue clear_color(
        std::array<float, 4>{{0.0f, 0.5451f, 0.5451f, 0.0f}});

    vk::ImageSubresourceRange image_subresource_range{
        vk::ImageAspectFlagBits::eColor,  // aspectMask
        0,                                // baseMipLevel
        1,                                // levelCount
        0,                                // baseArrayLayer
        1                                 // layerCount
    };

    for (size_t i = 0; i < m_graphics_queue_cmd_buffers.size(); i++) {
        vk::ImageMemoryBarrier barrier_from_present_to_clear{
            vk::AccessFlagBits::eMemoryRead,       // srcAccessMask
            vk::AccessFlagBits::eTransferWrite,    // dstAccessMask
            vk::ImageLayout::eUndefined,           // oldLayout
            vk::ImageLayout::eTransferDstOptimal,  // newLayout
            m_present_queue.index,                 // srcQueueFamilyIndex
            m_present_queue.index,                 // dstQueueFamilyIndex
            m_swapchain.images[i].handle,          // image
            image_subresource_range                // subresourceRange
        };

        vk::ImageMemoryBarrier barrier_from_clear_to_present{
            vk::AccessFlagBits::eTransferWrite,    // srcAccessMask
            vk::AccessFlagBits::eMemoryRead,       // dstAccessMask
            vk::ImageLayout::eTransferDstOptimal,  // oldLayout
            vk::ImageLayout::ePresentSrcKHR,       // newLayout
            m_present_queue.index,                 // srcQueueFamilyIndex
            m_present_queue.index,                 // dstQueueFamilyIndex
            m_swapchain.images[i].handle,          // image
            image_subresource_range                // subresourceRange
        };

        m_graphics_queue_cmd_buffers[i].begin(&cmd_buffer_begin_info);

        m_graphics_queue_cmd_buffers[i].pipelineBarrier(
            vk::PipelineStageFlagBits::eTransfer,
            vk::PipelineStageFlagBits::eTransfer, vk::DependencyFlags(), 0,
            nullptr, 0, nullptr, 1, &barrier_from_present_to_clear);

        m_graphics_queue_cmd_buffers[i].clearColorImage(
            m_swapchain.images[i].handle, vk::ImageLayout::eTransferDstOptimal,
            &clear_color, 1, &image_subresource_range);

        m_graphics_queue_cmd_buffers[i].pipelineBarrier(
            vk::PipelineStageFlagBits::eTransfer,
            vk::PipelineStageFlagBits::eBottomOfPipe, vk::DependencyFlags(), 0,
            nullptr, 0, nullptr, 1, &barrier_from_clear_to_present);

        result = m_graphics_queue_cmd_buffers[i].end();
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
    // If application wants to use more images at the same time it must ask
    // for more images
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
    // We can define other usage flags but we always need to check if they
    // are supported
    if (surface_capabilities.supportedUsageFlags &
        vk::ImageUsageFlagBits::eTransferDst) {
        return vk::ImageUsageFlagBits::eColorAttachment |
               vk::ImageUsageFlagBits::eTransferDst;
    }
    return vk::ImageUsageFlags(static_cast<vk::ImageUsageFlagBits>(-1));
}

vk::SurfaceTransformFlagBitsKHR Vk_RenderWindow::GetVulkanSwapChainTransform(
    const vk::SurfaceCapabilitiesKHR& surface_capabilities) {
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

void Vk_RenderWindow::ClearPipeline() {
    Vk_Context& context = Vk_Context::GetInstance();
    vk::Device& device = context.GetVulkanDevice();

    if (device) {
        device.waitIdle();

        if (m_graphics_queue_cmd_buffers.size() > 0 &&
            m_graphics_queue_cmd_buffers[0]) {
            device.freeCommandBuffers(
                m_graphics_queue_cmd_pool,
                static_cast<uint32>(m_graphics_queue_cmd_buffers.size()),
                m_graphics_queue_cmd_buffers.data());
            m_graphics_queue_cmd_buffers.clear();
        }

        if (m_graphics_queue_cmd_pool) {
            device.destroyCommandPool(m_graphics_queue_cmd_pool, nullptr);
            m_graphics_queue_cmd_pool = nullptr;
        }

        if (m_graphics_pipeline) {
            device.destroyPipeline(m_graphics_pipeline, nullptr);
            m_graphics_pipeline = nullptr;
        }

        if (m_render_pass) {
            device.destroyRenderPass(m_render_pass, nullptr);
            m_render_pass = nullptr;
        }

        for (size_t i = 0; i < m_framebuffers.size(); ++i) {
            if (m_framebuffers[i]) {
                device.destroyFramebuffer(m_framebuffers[i], nullptr);
                m_framebuffers[i] = nullptr;
            }
        }
        m_framebuffers.clear();
    }
}

bool Vk_RenderWindow::OnWindowSizeChanged() {
    // Update the base class attributes
    SDL_GetWindowSize(m_window, &m_size.x, &m_size.y);

    ClearPipeline();

    if (!CreateVulkanSwapChain()) {
        return false;
    }
    if (!CreateVulkanRenderPass()) {
        return false;
    }
    if (!CreateVulkanFrameBuffers()) {
        return false;
    }

    if (!CreateVulkanCommandBuffers()) {
        return false;
    }
    return true;
}

}  // namespace engine
