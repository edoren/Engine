#include <Graphics/3D/Camera.hpp>
#include <Renderer/UniformBufferObject.hpp>
#include <Renderer/Vertex.hpp>
#include <System/FileSystem.hpp>
#include <System/LogManager.hpp>
#include <System/StringFormat.hpp>

#include "Vk_RenderWindow.hpp"
#include "Vk_Shader.hpp"
#include "Vk_ShaderManager.hpp"
#include "Vk_Texture2D.hpp"
#include "Vk_TextureManager.hpp"

namespace engine {

namespace {

const String sTag("Vk_RenderWindow");

const uint32 sVertexBufferBindId(0);

const char* sShaderEntryPoint("main");

// Vulkan clip space has inverted Y and half Z.
const math::mat4 sClipMatrix = {
    {1.0F, 0.0F, 0.0F, 0.0F},
    {0.0F, -1.0F, 0.0F, 0.0F},
    {0.0F, 0.0F, 0.5F, 0.0F},
    {0.0F, 0.0F, 0.5F, 1.0F},
};

}  // namespace

Vk_RenderWindow::Vk_RenderWindow()
      : m_graphicsQueue(nullptr),
        m_presentQueue(nullptr),
        m_graphicsPipeline(VK_NULL_HANDLE),
        m_pipelineLayout(VK_NULL_HANDLE),
        m_renderPass(VK_NULL_HANDLE) {}

Vk_RenderWindow::~Vk_RenderWindow() {
    destroy();
}

bool Vk_RenderWindow::create(const String& name, const math::ivec2& size) {
    // Create the window
    math::ivec2 initial_pos(SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    uint32 window_flags(SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

    m_window = SDL_CreateWindow(name.getData(), initial_pos.x, initial_pos.y, size.x, size.y, window_flags);
    if (!m_window) {
        LogError(sTag, "SDL_CreateWindow fail: {}"_format(SDL_GetError()));
        return false;
    }

    // We assume that the graphics queue can also present
    Vk_Context& context = Vk_Context::GetInstance();
    m_graphicsQueue = &context.getGraphicsQueue();
    m_presentQueue = m_graphicsQueue;

    // Update the base class attributes
    RenderWindow::create(name, size);

    if (!m_surface.create(reinterpret_cast<SDL_Window*>(m_window))) {
        LogFatal(sTag, "Could not create the Surface");
        return false;
    }
    if (!checkWsiSupport()) {
        PhysicalDeviceParameters& physical_device = context.getPhysicalDevice();
        LogError(sTag, "Physical device {} doesn't include WSI support"_format(physical_device.properties.deviceName));
        return false;
    }

    if (!createDepthResources()) {
        LogError(sTag, "Could not create the DepthResources");
        return false;
    }

    if (!m_swapchain.create(m_surface, m_size.x, m_size.y)) {
        LogError(sTag, "Could not create the SwapChain");
        return false;
    }
    if (!createVulkanRenderPass()) {
        LogError(sTag, "Could not create the RenderPass");
        return false;
    }
    if (!createVulkanPipeline()) {
        LogError(sTag, "Could not create the Pipeline");
        return false;
    }

    m_renderResources.resize(m_swapchain.getImages().size());
    for (Vk_RenderResource& render_resource : m_renderResources) {
        if (!render_resource.create()) {
            LogError(sTag, "Could not create the RenderingResources");
            return false;
        }
    }

    return true;
}

void Vk_RenderWindow::destroy() {
    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.getVulkanDevice();

    if (device) {
        vkDeviceWaitIdle(device);

        m_renderResources.clear();

        if (m_graphicsPipeline) {
            vkDestroyPipeline(device, m_graphicsPipeline, nullptr);
            m_graphicsPipeline = VK_NULL_HANDLE;
        }

        if (m_pipelineLayout) {
            vkDestroyPipelineLayout(device, m_pipelineLayout, nullptr);
            m_pipelineLayout = VK_NULL_HANDLE;
        }

        if (m_renderPass) {
            vkDestroyRenderPass(device, m_renderPass, nullptr);
            m_renderPass = VK_NULL_HANDLE;
        }

        m_swapchain.destroy();
        m_commandWorkQueue.clear();
    }

    m_presentQueue = nullptr;
    m_graphicsQueue = nullptr;

    m_surface.destroy();

    RenderWindow::destroy();
}

void Vk_RenderWindow::resize(int width, int height) {
    RenderWindow::resize(width, height);
}

void Vk_RenderWindow::setFullScreen(bool fullscreen, bool isFake) {
    RenderWindow::setFullScreen(fullscreen, isFake);
}

void Vk_RenderWindow::setVSyncEnabled(bool /*vsync*/) {
    // if (SDL_GL_SetSwapInterval(vsync ? 1 : 0) == 0) {
    //     m_isVsyncEnabled = vsync;
    // } else {
    //     m_isVsyncEnabled = false;
    // }
}

void Vk_RenderWindow::swapBuffers() {
    if (m_swapchain.getHandle() == VK_NULL_HANDLE) {
        LogWarning(sTag, "SwapChain not avaliable");
        return;
    }

    static size_t sResourceIndex = 0;

    VkResult result = VK_SUCCESS;

    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.getVulkanDevice();

    Vk_RenderResource& current_rendering_resource = m_renderResources[sResourceIndex];
    uint32_t image_index;

    sResourceIndex = (sResourceIndex + 1) % m_swapchain.getImages().size();

    result = vkWaitForFences(device, 1, &current_rendering_resource.fence, VK_FALSE, 1000000000);
    if (result == VK_TIMEOUT) {
        LogError(sTag, "Waiting for fence takes too long");
        return;
    }

    vkResetFences(device, 1, &current_rendering_resource.fence);

    result = vkAcquireNextImageKHR(device, m_swapchain.getHandle(), UINT64_MAX,
                                   current_rendering_resource.imageAvailableSemaphore, VK_NULL_HANDLE, &image_index);
    switch (result) {
        case VK_SUCCESS:
        case VK_SUBOPTIMAL_KHR:
            break;
        case VK_ERROR_OUT_OF_DATE_KHR:
            onWindowResized(m_size);
            return;
        default:
            LogError(sTag, "Problem occurred during SwapChain image acquisition");
            return;
    }

    if (!prepareFrame(current_rendering_resource.commandBuffer, m_swapchain.getImages()[image_index],
                      current_rendering_resource.framebuffer)) {
        return;
    }

    VkPipelineStageFlags wait_dst_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submit_info = {
        VK_STRUCTURE_TYPE_SUBMIT_INFO,                          // sType
        nullptr,                                                // pNext
        1,                                                      // waitSemaphoreCount
        &current_rendering_resource.imageAvailableSemaphore,    // pWaitSemaphores
        &wait_dst_stage_mask,                                   // pWaitDstStageMask;
        1,                                                      // commandBufferCount
        &current_rendering_resource.commandBuffer,              // pCommandBuffers
        1,                                                      // signalSemaphoreCount
        &current_rendering_resource.finishedRenderingSemaphore  // pSignalSemaphores
    };

    result = vkQueueSubmit(m_graphicsQueue->getHandle(), 1, &submit_info, current_rendering_resource.fence);

    if (result != VK_SUCCESS) {
        LogError(sTag, "Error submitting the command buffers");
        return;
    }

    VkPresentInfoKHR present_info = {
        VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,                      // sType
        nullptr,                                                 // pNext
        1,                                                       // waitSemaphoreCount
        &current_rendering_resource.finishedRenderingSemaphore,  // pWaitSemaphores
        1,                                                       // swapchainCount
        &m_swapchain.getHandle(),                                // pSwapchains
        &image_index,                                            // pImageIndices
        nullptr                                                  // pResults
    };

    result = vkQueuePresentKHR(m_presentQueue->getHandle(), &present_info);

    switch (result) {
        case VK_SUCCESS:
            break;
        case VK_ERROR_OUT_OF_DATE_KHR:
        case VK_SUBOPTIMAL_KHR:
#if !PLATFORM_IS(PLATFORM_ANDROID)
            LogDebug(sTag, "VK_SUBOPTIMAL_KHR: Recreating SwapChain");
            onWindowResized(m_size);
#endif
            return;
        default:
            LogError(sTag, "Problem occurred during image presentation");
            return;
    }
}

void Vk_RenderWindow::clear(const Color& /*color*/) {  // RenderTarget
    // GL_CALL(glClearColor(color.r, color.g, color.b, color.a));
    // GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}

void Vk_RenderWindow::addCommandExecution(CommandType&& func) {
    if (isVisible()) {
        m_commandWorkQueue.push(std::move(func));
    }
}

void Vk_RenderWindow::submitGraphicsCommand(Function<void(VkCommandBuffer&)>&& func) {
    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.getVulkanDevice();

    VkResult result = VK_SUCCESS;

    // Prepare command buffer to copy data from staging buffer to the vertex
    // and index buffer
    VkCommandBufferAllocateInfo allocInfo = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,  // sType
        nullptr,                                         // pNext
        context.getGraphicsQueueCmdPool(),               // commandPool
        VK_COMMAND_BUFFER_LEVEL_PRIMARY,                 // level
        1                                                // commandBufferCount
    };

    VkCommandBuffer command_buffer = VK_NULL_HANDLE;
    result = vkAllocateCommandBuffers(device, &allocInfo, &command_buffer);
    if (result != VK_SUCCESS || command_buffer == VK_NULL_HANDLE) {
        LogError(sTag, "Could not allocate command buffer");
        return;
    }

    VkCommandBufferBeginInfo command_buffer_begin_info = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,  // sType
        nullptr,                                      // pNext
        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,  // flags
        nullptr                                       // pInheritanceInfo
    };

    vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info);

    // Call the function and pass the command buffer
    func(command_buffer);

    vkEndCommandBuffer(command_buffer);

    // Submit command buffer and copy data from staging buffer to the vertex
    // and index buffer
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

    result = vkQueueSubmit(context.getGraphicsQueue().getHandle(), 1, &submit_info, VK_NULL_HANDLE);
    if (result != VK_SUCCESS) {
        LogError(sTag, "Error submiting command buffer");
        return;
    }

    vkQueueWaitIdle(context.getGraphicsQueue().getHandle());
}

void Vk_RenderWindow::updateProjectionMatrix() {
    RenderWindow::updateProjectionMatrix();
    m_projection = sClipMatrix * m_projection;
}

bool Vk_RenderWindow::checkWsiSupport() {
    // Check that the device graphics queue family has WSI support
    VkBool32 wsi_support;
    Vk_Context& context = Vk_Context::GetInstance();
    PhysicalDeviceParameters& physical_device = context.getPhysicalDevice();
    QueueParameters& graphics_queue = context.getGraphicsQueue();

    vkGetPhysicalDeviceSurfaceSupportKHR(physical_device.getHandle(), graphics_queue.familyIndex, m_surface.getHandle(),
                                         &wsi_support);
    return wsi_support == VK_TRUE;
}

bool Vk_RenderWindow::createVulkanRenderPass() {
    VkResult result = VK_SUCCESS;

    // Create the attachment descriptions
    std::array<VkAttachmentDescription, 2> attachment_descriptions = {{
        {
            VkAttachmentDescriptionFlags(),    // flags
            m_swapchain.getFormat(),           // format
            VK_SAMPLE_COUNT_1_BIT,             // samples
            VK_ATTACHMENT_LOAD_OP_CLEAR,       // loadOp
            VK_ATTACHMENT_STORE_OP_STORE,      // storeOp
            VK_ATTACHMENT_LOAD_OP_DONT_CARE,   // stencilLoadOp
            VK_ATTACHMENT_STORE_OP_DONT_CARE,  // stencilStoreOp
            VK_IMAGE_LAYOUT_UNDEFINED,         // initialLayout
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR    // finalLayout
        },
        {
            VkAttachmentDescriptionFlags(),                    // flags
            m_depthFormat,                                     // format
            VK_SAMPLE_COUNT_1_BIT,                             // samples
            VK_ATTACHMENT_LOAD_OP_CLEAR,                       // loadOp
            VK_ATTACHMENT_STORE_OP_DONT_CARE,                  // storeOp
            VK_ATTACHMENT_LOAD_OP_DONT_CARE,                   // stencilLoadOp
            VK_ATTACHMENT_STORE_OP_DONT_CARE,                  // stencilStoreOp
            VK_IMAGE_LAYOUT_UNDEFINED,                         // initialLayout
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,  // finalLayout
        },
    }};

    std::array<VkAttachmentReference, 2> attachment_references = {{
        {
            0,                                        // attachment
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL  // layout
        },
        {
            1,                                                // attachment
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL  // layout
        },
    }};

    std::array<VkSubpassDescription, 1> subpass_descriptions = {{
        {
            VkSubpassDescriptionFlags(),      // flags
            VK_PIPELINE_BIND_POINT_GRAPHICS,  // pipelineBindPoint
            0,                                // inputAttachmentCount
            nullptr,                          // pInputAttachments
            1,                                // colorAttachmentCount
            &attachment_references[0],        // pColorAttachments
            nullptr,                          // pResolveAttachments
            &attachment_references[1],        // pDepthStencilAttachment
            0,                                // preserveAttachmentCount
            nullptr                           // pPreserveAttachments
        },
    }};

    std::array<VkSubpassDependency, 2> dependencies = {{
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
        },
    }};

    VkRenderPassCreateInfo render_pass_create_info = {
        VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,              // sType
        nullptr,                                                // pNext
        VkRenderPassCreateFlags(),                              // flags
        static_cast<uint32_t>(attachment_descriptions.size()),  // attachmentCount
        attachment_descriptions.data(),                         // pAttachments
        static_cast<uint32_t>(subpass_descriptions.size()),     // subpassCount
        subpass_descriptions.data(),                            // pSubpasses
        static_cast<uint32_t>(dependencies.size()),             // dependencyCount
        dependencies.data()                                     // pDependencies
    };

    // NOTES: Dependencies are important for performance

    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.getVulkanDevice();
    result = vkCreateRenderPass(device, &render_pass_create_info, nullptr, &m_renderPass);
    if (result != VK_SUCCESS) {
        LogError(sTag, "Could not create render pass");
        return false;
    }

    return true;
}

bool Vk_RenderWindow::createVulkanPipeline() {
    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.getVulkanDevice();

    VkResult result = VK_SUCCESS;

    Vk_Shader* shader = Vk_ShaderManager::GetInstance().getActiveShader();
    Vk_TextureManager& texture_manager = Vk_TextureManager::GetInstance();

    if (shader == nullptr) {
        LogError(sTag, "Active shader not set");
        return false;
    }

    if (!shader->getModule(ShaderType::VERTEX) || !shader->getModule(ShaderType::FRAGMENT)) {
        LogError(sTag, "Coud not get Vertex and/or Fragment shader module");
        return false;
    }

    std::array<VkVertexInputBindingDescription, 1> vertex_binding_descriptions = {{
        {
            sVertexBufferBindId,         // binding
            sizeof(Vertex),              // stride
            VK_VERTEX_INPUT_RATE_VERTEX  // inputRate
        },
    }};

    const Vk_VertexLayout& vertex_layout = shader->getVertexLayout();

    auto vertex_input_attrib_description = vertex_layout.getVertexInputAttributeDescription(sVertexBufferBindId);

    VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info = {
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,      // sType
        nullptr,                                                        // pNext
        0,                                                              // flags;
        static_cast<uint32_t>(vertex_binding_descriptions.size()),      // vertexBindingDescriptionCount
        vertex_binding_descriptions.data(),                             // pVertexBindingDescriptions
        static_cast<uint32_t>(vertex_input_attrib_description.size()),  // vertexAttributeDescriptionCount
        vertex_input_attrib_description.data()                          // pVertexAttributeDescriptions
    };

    std::array<VkPipelineShaderStageCreateInfo, 2> shader_stage_create_infos = {{
        // Vertex shader
        {
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,  // sType
            nullptr,                                              // pNext
            VkPipelineShaderStageCreateFlags(),                   // flags
            VK_SHADER_STAGE_VERTEX_BIT,                           // stage
            shader->getModule(ShaderType::VERTEX),                // module
            sShaderEntryPoint,                                    // pName
            nullptr                                               // pSpecializationInfo
        },
        // Fragment shader
        {
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,  // sType
            nullptr,                                              // pNext
            VkPipelineShaderStageCreateFlags(),                   // flags
            VK_SHADER_STAGE_FRAGMENT_BIT,                         // stage
            shader->getModule(ShaderType::FRAGMENT),              // module
            sShaderEntryPoint,                                    // pName
            nullptr                                               // pSpecializationInfo
        },
    }};

    VkPipelineInputAssemblyStateCreateInfo input_assembly_state_create_info = {
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,  // sType
        nullptr,                                                      // pNext
        VkPipelineInputAssemblyStateCreateFlags(),                    // flags
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,                          // topology
        VK_FALSE                                                      // primitiveRestartEnable
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
        VK_FALSE,                                                    // depthClampEnable
        VK_FALSE,                                                    // rasterizerDiscardEnable
        VK_POLYGON_MODE_FILL,                                        // polygonMode
        VK_CULL_MODE_NONE,                                           // cullMode
        VK_FRONT_FACE_COUNTER_CLOCKWISE,                             // frontFace
        VK_FALSE,                                                    // depthBiasEnable
        0.0F,                                                        // depthBiasConstantFactor
        0.0F,                                                        // depthBiasClamp
        0.0F,                                                        // depthBiasSlopeFactor
        1.0F                                                         // lineWidth
    };

    VkPipelineMultisampleStateCreateInfo multisample_state_create_info = {
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,  // sType
        nullptr,                                                   // pNext
        VkPipelineMultisampleStateCreateFlags(),                   // flags
        VK_SAMPLE_COUNT_1_BIT,                                     // rasterizationSamples
        VK_FALSE,                                                  // sampleShadingEnable
        1.0F,                                                      // minSampleShading
        nullptr,                                                   // pSampleMask
        VK_FALSE,                                                  // alphaToCoverageEnable
        VK_FALSE                                                   // alphaToOneEnable
    };

    VkPipelineColorBlendAttachmentState color_blend_attachment_state = {
        VK_FALSE,                             // blendEnable
        VK_BLEND_FACTOR_SRC_ALPHA,            // srcColorBlendFactor
        VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,  // dstColorBlendFactor
        VK_BLEND_OP_ADD,                      // colorBlendOp
        VK_BLEND_FACTOR_SRC_ALPHA,            // srcAlphaBlendFactor
        VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,  // dstAlphaBlendFactor
        VK_BLEND_OP_ADD,                      // alphaBlendOp
        (VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
         VK_COLOR_COMPONENT_A_BIT)  // colorWriteMask
    };

    VkPipelineDepthStencilStateCreateInfo depth_stencil_info = {
        VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,  // sType
        nullptr,                                                     // pNext
        VkPipelineDepthStencilStateCreateFlags(),                    // flags
        VK_TRUE,                                                     // depthTestEnable
        VK_TRUE,                                                     // depthWriteEnable
        VK_COMPARE_OP_LESS_OR_EQUAL,                                 // depthCompareOp
        VK_FALSE,                                                    // depthBoundsTestEnable
        VK_FALSE,                                                    // stencilTestEnable
        {},                                                          // front
        {},                                                          // back
        0.0F,                                                        // minDepthBounds
        1.0F,                                                        // maxDepthBounds
    };

    VkPipelineColorBlendStateCreateInfo color_blend_state_create_info = {
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,  // sType
        nullptr,                                                   // pNext
        VkPipelineColorBlendStateCreateFlags(),                    // flags
        VK_FALSE,                                                  // logicOpEnable
        VK_LOGIC_OP_COPY,                                          // logicOp
        1,                                                         // attachmentCount
        &color_blend_attachment_state,                             // pAttachments
        {0.0F, 0.0F, 0.0F, 0.0F}                                   // blendConstants[4]
    };

    // Define the pipeline dynamic states
    std::array<VkDynamicState, 2> dynamic_states = {{
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    }};
    VkPipelineDynamicStateCreateInfo dynamic_state_create_info = {
        VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,  // sType
        nullptr,                                               // pNext
        0,                                                     // flags
        static_cast<uint32_t>(dynamic_states.size()),          // dynamicStateCount
        dynamic_states.data()                                  // pDynamicStates
    };

    // Create the PipelineLayout
    std::array<VkDescriptorSetLayout, 2> descriptor_set_layouts = {{
        shader->getUboDescriptorSetLayout(),
        texture_manager.getDescriptorSetLayout(),
    }};

    VkPipelineLayoutCreateInfo layout_create_info = {
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,         // sType
        nullptr,                                               // pNext
        VkPipelineLayoutCreateFlags(),                         // flags
        static_cast<uint32_t>(descriptor_set_layouts.size()),  // setLayoutCount
        descriptor_set_layouts.data(),                         // pSetLayouts
        0,                                                     // pushConstantRangeCount
        nullptr                                                // pPushConstantRanges
    };

    result = vkCreatePipelineLayout(device, &layout_create_info, nullptr, &m_pipelineLayout);
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
        &vertex_input_state_create_info,                          // pVertexInputState;
        &input_assembly_state_create_info,                        // pInputAssemblyState
        nullptr,                                                  // pTessellationState
        &viewport_state_create_info,                              // pViewportState
        &rasterization_state_create_info,                         // pRasterizationState
        &multisample_state_create_info,                           // pMultisampleState
        &depth_stencil_info,                                      // pDepthStencilState
        &color_blend_state_create_info,                           // pColorBlendState
        &dynamic_state_create_info,                               // pDynamicState
        m_pipelineLayout,                                         // layout
        m_renderPass,                                             // renderPass
        0,                                                        // subpass
        VK_NULL_HANDLE,                                           // basePipelineHandle
        -1                                                        // basePipelineIndex
    };

    result =
        vkCreateGraphicsPipelines(device, VkPipelineCache(), 1, &pipeline_create_info, nullptr, &m_graphicsPipeline);
    if (result != VK_SUCCESS) {
        LogError(sTag, "Could not create graphics pipeline");
        vkDestroyPipelineLayout(device, m_pipelineLayout, nullptr);
        m_pipelineLayout = VK_NULL_HANDLE;
        return false;
    }

    return true;
}

bool Vk_RenderWindow::createVulkanFrameBuffer(VkFramebuffer& framebuffer, VkImageView& imageView) {
    VkResult result = VK_SUCCESS;

    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.getVulkanDevice();

    if (framebuffer) {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }

    std::array<VkImageView, 2> attachments = {
        {imageView, m_depthImage.getView()},
    };

    VkFramebufferCreateInfo framebuffer_create_info = {
        VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,  // sType
        nullptr,                                    // pNext
        VkFramebufferCreateFlags(),                 // flags
        m_renderPass,                               // renderPass
        static_cast<uint32_t>(attachments.size()),  // attachmentCount
        attachments.data(),                         // pAttachments
        static_cast<uint32_t>(m_size.x),            // width
        static_cast<uint32_t>(m_size.y),            // height
        1                                           // layers
    };

    result = vkCreateFramebuffer(device, &framebuffer_create_info, nullptr, &framebuffer);
    if (result != VK_SUCCESS) {
        LogError(sTag, "Could not create a framebuffer");
        return false;
    }

    return true;
}

bool Vk_RenderWindow::prepareFrame(VkCommandBuffer commandBuffer, Vk_Image& image, VkFramebuffer& framebuffer) {
    VkResult result = VK_SUCCESS;

    if (!createVulkanFrameBuffer(framebuffer, image.getView())) {
        return false;
    }

    VkCommandBufferBeginInfo command_buffer_begin_info = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,  // sType
        nullptr,                                      // pNext
        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,  // flags
        nullptr                                       // pInheritanceInfo
    };

    vkBeginCommandBuffer(commandBuffer, &command_buffer_begin_info);

    VkImageSubresourceRange image_subresource_range = {
        VK_IMAGE_ASPECT_COLOR_BIT,  // aspectMask
        0,                          // baseMipLevel
        1,                          // levelCount
        0,                          // baseArrayLayer
        1                           // layerCount
    };

    if (m_presentQueue->getHandle() != m_graphicsQueue->getHandle()) {
        VkImageMemoryBarrier barrier_from_present_to_draw = {
            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,  // sType
            nullptr,                                 // pNext
            VK_ACCESS_MEMORY_READ_BIT,               // srcAccessMask
            VK_ACCESS_MEMORY_READ_BIT,               // dstAccessMask
            VK_IMAGE_LAYOUT_UNDEFINED,               // oldLayout
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,         // newLayout
            m_presentQueue->familyIndex,             // srcQueueFamilyIndex
            m_graphicsQueue->familyIndex,            // dstQueueFamilyIndex
            image.getHandle(),                       // image
            image_subresource_range                  // subresourceRange
        };
        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                             VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1,
                             &barrier_from_present_to_draw);
    }

    std::array<VkClearValue, 2> clear_values = {};
    clear_values[0].color = {{0.0F, 0.0F, 0.0F, 0.0F}};
    clear_values[1].depthStencil = {1.0F, 0};

    VkRenderPassBeginInfo render_pass_begin_info = {
        VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,  // sType
        nullptr,                                   // pNext
        m_renderPass,                              // renderPass
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
        static_cast<uint32_t>(clear_values.size()),  // clearValueCount
        clear_values.data()                          // pClearValues
    };

    vkCmdBeginRenderPass(commandBuffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport = {
        0.0F,                          // x
        0.0F,                          // y
        static_cast<float>(m_size.x),  // width
        static_cast<float>(m_size.y),  // height
        0.0F,                          // minDepth
        1.0F                           // maxDepth
    };

    VkRect2D scissor = {
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
    };

    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);

    Vk_Shader* shader = Vk_ShaderManager::GetInstance().getActiveShader();

    // Update static uniform buffer
    const Camera* active_camera = getActiveCamera();

    math::vec3 front_vector;
    math::vec3 light_position;  // TMP: Get this from other
                                //      part a LightManager maybe?
    if (active_camera != nullptr) {
        front_vector = active_camera->getFrontVector();
        light_position = active_camera->getPosition();
    }

    UniformBufferObject& ubo = shader->getUbo();
    ubo.setAttributeValue("cameraFront", front_vector);
    ubo.setAttributeValue("lightPosition", light_position);
    ///

    uint32 index = 0;
    while (m_commandWorkQueue.getSize() > 0) {
        auto task = m_commandWorkQueue.pop();
        task(index++, commandBuffer, m_pipelineLayout);
    }

    shader->uploadUniformBuffers();

    vkCmdEndRenderPass(commandBuffer);

    if (m_graphicsQueue->getHandle() != m_presentQueue->getHandle()) {
        VkImageMemoryBarrier barrier_from_draw_to_present = {
            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,  // sType
            nullptr,                                 // pNext
            VK_ACCESS_MEMORY_READ_BIT,               // srcAccessMask
            VK_ACCESS_MEMORY_READ_BIT,               // dstAccessMask
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,         // oldLayout
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,         // newLayout
            m_graphicsQueue->familyIndex,            // srcQueueFamilyIndex
            m_presentQueue->familyIndex,             // dstQueueFamilyIndex
            image.getHandle(),                       // image
            image_subresource_range                  // subresourceRange
        };
        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                             VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1,
                             &barrier_from_draw_to_present);
    }

    result = vkEndCommandBuffer(commandBuffer);
    if (result != VK_SUCCESS) {
        LogError(sTag, "Could not record command buffer");
        return false;
    }

    return true;
}

bool Vk_RenderWindow::createDepthResources() {
    Vk_Context& context = Vk_Context::GetInstance();
    PhysicalDeviceParameters& physical_device = context.getPhysicalDevice();

    auto lFindSupportedFormat = [&physical_device](const std::vector<VkFormat>& candidates, VkImageTiling tiling,
                                                   VkFormatFeatureFlags features) -> VkFormat {
        for (VkFormat format : candidates) {
            VkFormatProperties properties = physical_device.getFormatProperties(format);
            switch (tiling) {
                case VK_IMAGE_TILING_LINEAR:
                    if ((properties.linearTilingFeatures & features) == features) {
                        return format;
                    }
                    break;
                case VK_IMAGE_TILING_OPTIMAL:
                    if ((properties.optimalTilingFeatures & features) == features) {
                        return format;
                    }
                    break;
                default:
                    break;
            }
        }

        return VK_FORMAT_UNDEFINED;
    };

    m_depthFormat =
        lFindSupportedFormat({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
                             VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

    if (m_depthFormat == VK_FORMAT_UNDEFINED) {
        LogError(sTag, "Supported Depth format not found");
        return false;
    }

    m_depthImage.destroy();

    if (!m_depthImage.createImage(math::uvec2(m_size), m_depthFormat, VK_IMAGE_TILING_OPTIMAL,
                                  VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
        LogError(sTag, "Could not create depth image");
        return false;
    }

    if (!m_depthImage.allocateMemory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
        LogError(sTag, "Could not allocate memory for depth image");
        return false;
    }

    if (!m_depthImage.createImageView(m_depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT)) {
        LogError(sTag, "Could not create depth image view");
        return false;
    }

    submitGraphicsCommand([this](VkCommandBuffer& commandBuffer) {
        bool has_stencil_component =
            (m_depthFormat == VK_FORMAT_D32_SFLOAT_S8_UINT || m_depthFormat == VK_FORMAT_D24_UNORM_S8_UINT);

        VkImageAspectFlags aspect_mask = VK_IMAGE_ASPECT_DEPTH_BIT;
        if (has_stencil_component) {
            aspect_mask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }

        VkImageMemoryBarrier depth_barrier = {
            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,  // sType
            nullptr,                                 // pNext
            0,                                       // srcAccessMask
            (VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
             VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT),    // dstAccessMask
            VK_IMAGE_LAYOUT_UNDEFINED,                         // oldLayout
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,  // newLayout
            VK_QUEUE_FAMILY_IGNORED,                           // srcQueueFamilyIndex
            VK_QUEUE_FAMILY_IGNORED,                           // dstQueueFamilyIndex
            m_depthImage.getHandle(),                          // image
            {
                aspect_mask,  // aspectMask
                0,            // baseMipLevel
                1,            // levelCount
                0,            // baseArrayLayer
                1             // layerCount
            }                 // subresourceRange
        };

        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                             VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, 0, 0, nullptr, 0, nullptr, 1, &depth_barrier);

        LogInfo(sTag, "Depth resources created with dimensions [{}, {}]"_format(m_size.x, m_size.y));
    });

    return true;
}

void Vk_RenderWindow::onWindowResized(const math::ivec2& size) {
    if (m_surface.getHandle() != VK_NULL_HANDLE) {
        // Recreate the depth image
        createDepthResources();
        // Recreate the Vulkan Swapchain
        m_swapchain.create(m_surface, size.x, size.y);
        m_commandWorkQueue.clear();
    }
}

void Vk_RenderWindow::onAppWillEnterBackground() {
    RenderWindow::onAppWillEnterBackground();
    vkQueueWaitIdle(m_graphicsQueue->getHandle());
    m_renderResources.clear();
    m_swapchain.destroy();
    m_surface.destroy();
    m_commandWorkQueue.clear();
}

void Vk_RenderWindow::onAppDidEnterBackground() {
    RenderWindow::onAppDidEnterBackground();
}

void Vk_RenderWindow::onAppWillEnterForeground() {
    RenderWindow::onAppWillEnterForeground();
}

void Vk_RenderWindow::onAppDidEnterForeground() {
    RenderWindow::onAppDidEnterForeground();
    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.getVulkanDevice();

    if (device) {
        vkDeviceWaitIdle(device);

        if (!m_surface.create(reinterpret_cast<SDL_Window*>(m_window))) {
            LogFatal(sTag, "Could not create the Surface");
            return;
        }

        if (!checkWsiSupport()) {
            PhysicalDeviceParameters& physical_device = context.getPhysicalDevice();
            LogFatal(sTag,
                     "Physical device {} doesn't include WSI "
                     "support"_format(physical_device.properties.deviceName));
            return;
        }

        if (!createDepthResources()) {
            LogFatal(sTag, "Could not create the DepthResources");
            return;
        }

        if (!m_swapchain.create(m_surface, m_size.x, m_size.y)) {
            LogFatal(sTag, "Could not create the SwapChain");
            return;
        }

        m_renderResources.resize(m_swapchain.getImages().size());
        for (Vk_RenderResource& render_resource : m_renderResources) {
            if (!render_resource.create()) {
                LogFatal(sTag, "Could not create the RenderingResources");
                return;
            }
        }
    }
}

}  // namespace engine
