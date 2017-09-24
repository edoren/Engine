#include <System/FileSystem.hpp>
#include <System/LogManager.hpp>
#include <System/StringFormat.hpp>

#include "Vk_RenderWindow.hpp"
#include "Vk_Shader.hpp"

namespace engine {

namespace {

const String sTag("Vk_RenderWindow");

const size_t sResourceCount(3);

const size_t sStagingBufferSize(4000);

}  // namespace

struct VertexData {
    math::Vector4<float> position;
    math::Vector4<float> color;
};

Vk_RenderWindow::Vk_RenderWindow()
      : m_window(nullptr),
        m_surface(),
        m_graphics_queue(nullptr),
        m_present_queue(nullptr),
        m_swapchain(),
        m_graphics_pipeline(VK_NULL_HANDLE),
        m_graphics_queue_cmd_pool(VK_NULL_HANDLE),
        m_render_pass(VK_NULL_HANDLE),
        m_vertex_buffer(),
        m_staging_buffer(),
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
        LogError(sTag, "SDL_CreateWindow fail: {}"_format(SDL_GetError()));
        return false;
    }

    // We assume that the graphics queue can also present
    Vk_Context& context = Vk_Context::GetInstance();
    m_graphics_queue = &context.GetGraphicsQueue();
    m_present_queue = m_graphics_queue;

    // Update the base class attributes
    SDL_GetWindowSize(m_window, &m_size.x, &m_size.y);
    m_name = name;

    if (!m_surface.Create(m_window)) {
        LogFatal(sTag, "Could not create the Surface");
        return false;
    }
    if (!CheckWSISupport()) {
        PhysicalDeviceParameters& physical_device = context.GetPhysicalDevice();
        LogError(sTag,
                 "Physical device {} doesn't include WSI "
                 "support"_format(physical_device.properties.deviceName));
        return false;
    }

    if (!m_swapchain.Create(m_surface, m_size.x, m_size.y)) {
        LogError(sTag, "Could not create the SwapChain");
        return false;
    }
    if (!CreateVulkanRenderPass()) {
        LogError(sTag, "Could not create the RenderPass");
        return false;
    }
    if (!CreateVulkanPipeline()) {
        LogError(sTag, "Could not create the Pipeline");
        return false;
    }

    if (!CreateRenderingResources()) {
        LogError(sTag, "Could not create the RenderingResources");
        return false;
    }

    if (!CreateVulkanVertexBuffer()) {
        LogError(sTag, "Could not create the VertexBuffer");
        return false;
    }

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

        m_vertex_buffer.Destroy();

        m_staging_buffer.Destroy();

        if (m_graphics_pipeline) {
            vkDestroyPipeline(device, m_graphics_pipeline, nullptr);
            m_graphics_pipeline = VK_NULL_HANDLE;
        }

        if (m_render_pass) {
            vkDestroyRenderPass(device, m_render_pass, nullptr);
            m_render_pass = VK_NULL_HANDLE;
        }

        m_swapchain.Destroy();
    }

    m_present_queue = nullptr;
    m_graphics_queue = nullptr;

    m_surface.Destroy();

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
    if (m_swapchain.GetHandle() == VK_NULL_HANDLE) {
        LogWarning(sTag, "SwapChain not avaliable");
        return;
    }

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
        device, m_swapchain.GetHandle(), UINT64_MAX,
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
                      m_swapchain.GetImages()[image_index],
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

    result = vkQueueSubmit(m_graphics_queue->handle, 1, &submit_info,
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
        &m_swapchain.GetHandle(),            // pSwapchains
        &image_index,                        // pImageIndices
        nullptr                              // pResults
    };

    result = vkQueuePresentKHR(m_present_queue->handle, &present_info);

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

bool Vk_RenderWindow::CheckWSISupport() {
    // Check that the device graphics queue family has WSI support
    VkBool32 wsi_support;
    Vk_Context& context = Vk_Context::GetInstance();
    PhysicalDeviceParameters& physical_device = context.GetPhysicalDevice();
    QueueParameters& graphics_queue = context.GetGraphicsQueue();

    vkGetPhysicalDeviceSurfaceSupportKHR(physical_device.handle,
                                         graphics_queue.family_index,
                                         m_surface.GetHandle(), &wsi_support);
    return wsi_support == VK_TRUE;
}

bool Vk_RenderWindow::CreateVulkanSemaphore(VkSemaphore* semaphore) {
    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.GetVulkanDevice();

    VkSemaphoreCreateInfo semaphore_create_info = {
        VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,  // sType
        nullptr,                                  // pNext
        VkSemaphoreCreateFlags()                  // flags
    };

    VkResult result =
        vkCreateSemaphore(device, &semaphore_create_info, nullptr, semaphore);
    if (result != VK_SUCCESS) {
        LogError(sTag, "Could not create semaphore");
        return false;
    }

    return true;
}

bool Vk_RenderWindow::CreateVulkanFence(VkFenceCreateFlags flags,
                                        VkFence* fence) {
    VkResult result = VK_SUCCESS;

    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.GetVulkanDevice();

    VkFenceCreateInfo fence_create_info = {
        VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,  // sType
        nullptr,                              // pNext
        flags                                 // flags
    };

    result = vkCreateFence(device, &fence_create_info, nullptr, fence);
    if (result != VK_SUCCESS) {
        LogError(sTag, "Could not create fence");
        return false;
    }

    return true;
}

bool Vk_RenderWindow::CreateVulkanCommandPool(QueueParameters& queue,
                                              VkCommandPool* cmd_pool) {
    VkResult result = VK_SUCCESS;

    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.GetVulkanDevice();

    // Create the pool for the command buffers
    VkCommandPoolCreateInfo cmd_pool_create_info = {
        VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,  // sType
        nullptr,                                     // pNext
        (VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT |
         VK_COMMAND_POOL_CREATE_TRANSIENT_BIT),  // flags
        queue.family_index                       // queueFamilyIndex
    };

    result =
        vkCreateCommandPool(device, &cmd_pool_create_info, nullptr, cmd_pool);
    if (result != VK_SUCCESS) {
        LogError(sTag, "Could not create a command pool");
        return false;
    }

    return true;
}

bool Vk_RenderWindow::AllocateVulkanCommandBuffers(
    VkCommandPool& cmd_pool, uint32_t count, VkCommandBuffer* command_buffer) {
    VkResult result = VK_SUCCESS;

    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.GetVulkanDevice();

    // Allocate space in the pool for the buffer
    VkCommandBufferAllocateInfo cmd_buffer_allocate_info = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,  // sType
        nullptr,                                         // pNext
        cmd_pool,                                        // commandPool
        VK_COMMAND_BUFFER_LEVEL_PRIMARY,                 // level
        count                                            // bufferCount
    };
    result = vkAllocateCommandBuffers(device, &cmd_buffer_allocate_info,
                                      command_buffer);
    if (result != VK_SUCCESS) {
        LogError(sTag, "Could not allocate command buffer");
        return false;
    }

    return true;
}

bool Vk_RenderWindow::CreateVulkanRenderPass() {
    VkResult result = VK_SUCCESS;

    // Create the attachment descriptions
    VkAttachmentDescription attachment_descriptions[] = {{
        VkAttachmentDescriptionFlags(),    // flags
        m_swapchain.GetFormat(),           // format
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

        FileSystem& fs = FileSystem::GetInstance();

        fs.LoadFileData("shaders/spirv/triangle.vert", &vertex_shader_code);
        fs.LoadFileData("shaders/spirv/triangle.frag", &fragment_shader_code);

        vertex_shader_module.LoadFromMemory(vertex_shader_code.data(),
                                            vertex_shader_code.size(),
                                            ShaderType::eVertex);
        fragment_shader_module.LoadFromMemory(fragment_shader_code.data(),
                                              fragment_shader_code.size(),
                                              ShaderType::eFragment);
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

    if (!m_vertex_buffer.Create(sizeof(vertex_data),
                                (VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                                 VK_BUFFER_USAGE_TRANSFER_DST_BIT),
                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
        LogFatal(sTag, "Could not create Vertex Buffer");
    }

    if (!m_staging_buffer.Create(sStagingBufferSize,
                                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
        LogFatal(sTag, "Could not create Staging Buffer");
    }

    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.GetVulkanDevice();

    void* vertex_buffer_memory_pointer;
    result = vkMapMemory(device, m_staging_buffer.GetMemory(), 0,
                         m_staging_buffer.GetSize(), 0,
                         &vertex_buffer_memory_pointer);
    if (result != VK_SUCCESS) {
        LogError(sTag,
                 "Could not map memory and upload data to a vertex buffer");
        return false;
    }

    std::memcpy(vertex_buffer_memory_pointer, vertex_data,
                static_cast<size_t>(m_vertex_buffer.GetSize()));

    VkMappedMemoryRange flush_range = {
        VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,  // sType
        nullptr,                                // pNext
        m_staging_buffer.GetMemory(),           // memory
        0,                                      // offset
        m_vertex_buffer.GetSize()               // size
    };
    vkFlushMappedMemoryRanges(device, 1, &flush_range);

    vkUnmapMemory(device, m_staging_buffer.GetMemory());

    // Prepare command buffer to copy data from staging buffer to a vertex
    // buffer
    VkCommandBufferBeginInfo command_buffer_begin_info = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,  // sType
        nullptr,                                      // pNext
        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,  // flags
        nullptr                                       // pInheritanceInfo
    };

    VkCommandBuffer command_buffer = m_render_resources[0].command_buffer;

    vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info);

    VkBufferCopy buffer_copy_info = {
        0,                         // srcOffset
        0,                         // dstOffset
        m_vertex_buffer.GetSize()  // size
    };
    vkCmdCopyBuffer(command_buffer, m_staging_buffer.GetHandle(),
                    m_vertex_buffer.GetHandle(), 1, &buffer_copy_info);

    VkBufferMemoryBarrier buffer_memory_barrier = {
        VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,  // sType;
        nullptr,                                  // pNext
        VK_ACCESS_MEMORY_WRITE_BIT,               // srcAccessMask
        VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,      // dstAccessMask
        VK_QUEUE_FAMILY_IGNORED,                  // srcQueueFamilyIndex
        VK_QUEUE_FAMILY_IGNORED,                  // dstQueueFamilyIndex
        m_vertex_buffer.GetHandle(),              // buffer
        0,                                        // offset
        VK_WHOLE_SIZE                             // size
    };
    vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, 0, 0, nullptr, 1,
                         &buffer_memory_barrier, 0, nullptr);

    vkEndCommandBuffer(command_buffer);

    // Submit command buffer and copy data from staging buffer to a vertex
    // buffer
    VkSubmitInfo submit_info = {
        VK_STRUCTURE_TYPE_SUBMIT_INFO,  // sType
        nullptr,                        // pNext
        0,                              // waitSemaphoreCount
        nullptr,                        // pWaitSemaphores
        nullptr,                        // pWaitDstStageMask;
        1,                              // commandBufferCount
        &command_buffer,                // pCommandBuffers
        0,                              // signalSemaphoreCount
        nullptr                         // pSignalSemaphores
    };

    result = vkQueueSubmit(m_graphics_queue->handle, 1, &submit_info,
                           VK_NULL_HANDLE);
    if (result != VK_SUCCESS) {
        return false;
    }

    vkDeviceWaitIdle(device);

    return true;
}

bool Vk_RenderWindow::CreateRenderingResources() {
    if (!CreateVulkanCommandPool(*m_graphics_queue,
                                 &m_graphics_queue_cmd_pool)) {
        return false;
    }

    for (size_t i = 0; i < m_render_resources.size(); ++i) {
        if (!AllocateVulkanCommandBuffers(
                m_graphics_queue_cmd_pool, 1,
                &m_render_resources[i].command_buffer) ||
            !CreateVulkanSemaphore(
                &m_render_resources[i].image_available_semaphore) ||
            !CreateVulkanSemaphore(
                &m_render_resources[i].finished_rendering_semaphore) ||
            !CreateVulkanFence(VK_FENCE_CREATE_SIGNALED_BIT,
                               &m_render_resources[i].fence)) {
            return false;
        }
    }

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

    if (m_present_queue->handle != m_graphics_queue->handle) {
        VkImageMemoryBarrier barrier_from_present_to_draw = {
            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,  // sType
            nullptr,                                 // pNext
            VK_ACCESS_MEMORY_READ_BIT,               // srcAccessMask
            VK_ACCESS_MEMORY_READ_BIT,               // dstAccessMask
            VK_IMAGE_LAYOUT_UNDEFINED,               // oldLayout
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,         // newLayout
            m_present_queue->family_index,           // srcQueueFamilyIndex
            m_graphics_queue->family_index,          // dstQueueFamilyIndex
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
    vkCmdBindVertexBuffers(command_buffer, 0, 1, &m_vertex_buffer.GetHandle(),
                           &offset);

    vkCmdDraw(command_buffer, 4, 1, 0, 0);

    vkCmdEndRenderPass(command_buffer);

    if (m_graphics_queue->handle != m_present_queue->handle) {
        VkImageMemoryBarrier barrier_from_draw_to_present = {
            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,  // sType
            nullptr,                                 // pNext
            VK_ACCESS_MEMORY_READ_BIT,               // srcAccessMask
            VK_ACCESS_MEMORY_READ_BIT,               // dstAccessMask
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,         // oldLayout
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,         // newLayout
            m_graphics_queue->family_index,          // srcQueueFamilyIndex
            m_present_queue->family_index,           // dstQueueFamilyIndex
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
    m_swapchain.Create(m_surface, m_size.x, m_size.y);
}

void Vk_RenderWindow::OnAppWillEnterBackground() {
    Vk_Context& context = Vk_Context::GetInstance();
    VkInstance& instance = context.GetVulkanInstance();

    m_swapchain.Destroy();
    m_surface.Destroy();
}

void Vk_RenderWindow::OnAppDidEnterBackground() {}

void Vk_RenderWindow::OnAppWillEnterForeground() {}

void Vk_RenderWindow::OnAppDidEnterForeground() {
    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.GetVulkanDevice();

    if (device) {
        vkDeviceWaitIdle(device);

        if (!m_surface.Create(m_window)) {
            LogFatal(sTag, "Could not create the Surface");
            return;
        }

        if (!CheckWSISupport()) {
            PhysicalDeviceParameters& physical_device = context.GetPhysicalDevice();
            LogFatal(sTag,
                     "Physical device {} doesn't include WSI "
                             "support"_format(physical_device.properties.deviceName));
            return;
        }

        if (!m_swapchain.Create(m_surface, m_size.x, m_size.y)) {
            LogFatal(sTag, "Could not create the SwapChain");
            return;
        }
    }
}

}  // namespace engine
