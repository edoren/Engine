#include <Graphics/3D/Camera.hpp>
#include <Renderer/Drawable.hpp>
#include <Renderer/RenderStates.hpp>
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
    {1.0f, 0.0f, 0.0f, 0.0f},
    {0.0f, -1.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, 0.5f, 0.0f},
    {0.0f, 0.0f, 0.5f, 1.0f},
};

}  // namespace

Vk_RenderWindow::Vk_RenderWindow()
      : m_surface(),
        m_graphics_queue(nullptr),
        m_present_queue(nullptr),
        m_swapchain(),
        m_graphics_pipeline(VK_NULL_HANDLE),
        m_pipeline_layout(VK_NULL_HANDLE),
        m_render_pass(VK_NULL_HANDLE),
        m_render_resources() {}

Vk_RenderWindow::~Vk_RenderWindow() {
    Destroy();
}

bool Vk_RenderWindow::Create(const String& name, const math::ivec2& size) {
    // Create the window
    math::ivec2 initial_pos(SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    uint32 window_flags(SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN |
                        SDL_WINDOW_RESIZABLE);

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
    RenderWindow::Create(name, size);

    if (!m_surface.Create(reinterpret_cast<SDL_Window*>(m_window))) {
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

    if (!CreateDepthResources()) {
        LogError(sTag, "Could not create the DepthResources");
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

    return true;
}

void Vk_RenderWindow::Destroy() {
    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.GetVulkanDevice();

    if (device) {
        vkDeviceWaitIdle(device);

        for (size_t i = 0; i < m_render_resources.size(); i++) {
            if (m_render_resources[i].framebuffer) {
                vkDestroyFramebuffer(device, m_render_resources[i].framebuffer,
                                     nullptr);
            }
            if (m_render_resources[i].command_buffer) {
                vkFreeCommandBuffers(device, context.GetGraphicsQueueCmdPool(),
                                     1, &m_render_resources[i].command_buffer);
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

        if (m_graphics_pipeline) {
            vkDestroyPipeline(device, m_graphics_pipeline, nullptr);
            m_graphics_pipeline = VK_NULL_HANDLE;
        }

        if (m_pipeline_layout) {
            vkDestroyPipelineLayout(device, m_pipeline_layout, nullptr);
            m_pipeline_layout = VK_NULL_HANDLE;
        }

        if (m_render_pass) {
            vkDestroyRenderPass(device, m_render_pass, nullptr);
            m_render_pass = VK_NULL_HANDLE;
        }

        m_swapchain.Destroy();
        m_command_work_queue.Clear();
    }

    m_present_queue = nullptr;
    m_graphics_queue = nullptr;

    m_surface.Destroy();

    RenderWindow::Destroy();
}

void Vk_RenderWindow::Resize(int width, int height) {
    RenderWindow::Resize(width, height);
}

void Vk_RenderWindow::SetFullScreen(bool fullscreen, bool is_fake) {
    RenderWindow::SetFullScreen(fullscreen, is_fake);
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

    resource_index = (resource_index + 1) % m_swapchain.GetImages().size();

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

    result = vkQueueSubmit(m_graphics_queue->GetHandle(), 1, &submit_info,
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

    result = vkQueuePresentKHR(m_present_queue->GetHandle(), &present_info);

    switch (result) {
        case VK_SUCCESS:
            break;
        case VK_ERROR_OUT_OF_DATE_KHR:
        case VK_SUBOPTIMAL_KHR:
            LogDebug(sTag, "Recreating SwapChain");
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

void Vk_RenderWindow::AddCommandExecution(CommandType&& func) {
    if (IsVisible()) {
        m_command_work_queue.Push(std::move(func));
    }
}

void Vk_RenderWindow::SubmitGraphicsCommand(
    Function<void(VkCommandBuffer&)>&& func) {
    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.GetVulkanDevice();

    VkResult result = VK_SUCCESS;

    // Prepare command buffer to copy data from staging buffer to the vertex
    // and index buffer
    VkCommandBufferAllocateInfo allocInfo = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,  // sType
        nullptr,                                         // pNext
        context.GetGraphicsQueueCmdPool(),               // commandPool
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

    result = vkQueueSubmit(context.GetGraphicsQueue().GetHandle(), 1,
                           &submit_info, VK_NULL_HANDLE);
    if (result != VK_SUCCESS) {
        LogError(sTag, "Error submiting command buffer");
        return;
    }

    vkQueueWaitIdle(context.GetGraphicsQueue().GetHandle());
}

void Vk_RenderWindow::UpdateProjectionMatrix() {
    RenderWindow::UpdateProjectionMatrix();
    m_projection = sClipMatrix * m_projection;
}

bool Vk_RenderWindow::CheckWSISupport() {
    // Check that the device graphics queue family has WSI support
    VkBool32 wsi_support;
    Vk_Context& context = Vk_Context::GetInstance();
    PhysicalDeviceParameters& physical_device = context.GetPhysicalDevice();
    QueueParameters& graphics_queue = context.GetGraphicsQueue();

    vkGetPhysicalDeviceSurfaceSupportKHR(physical_device.GetHandle(),
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
    std::array<VkAttachmentDescription, 2> attachment_descriptions = {{
        {
            VkAttachmentDescriptionFlags(),    // flags
            m_swapchain.GetFormat(),           // format
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
            m_depth_format,                                    // format
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
        VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,  // sType
        nullptr,                                    // pNext
        VkRenderPassCreateFlags(),                  // flags
        static_cast<uint32_t>(
            attachment_descriptions.size()),                 // attachmentCount
        attachment_descriptions.data(),                      // pAttachments
        static_cast<uint32_t>(subpass_descriptions.size()),  // subpassCount
        subpass_descriptions.data(),                         // pSubpasses
        static_cast<uint32_t>(dependencies.size()),          // dependencyCount
        dependencies.data()                                  // pDependencies
    };

    // NOTES: Dependencies are important for performance

    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.GetVulkanDevice();
    result = vkCreateRenderPass(device, &render_pass_create_info, nullptr,
                                &m_render_pass);
    if (result != VK_SUCCESS) {
        LogError(sTag, "Could not create render pass");
        return false;
    }

    return true;
}

bool Vk_RenderWindow::CreateVulkanPipeline() {
    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.GetVulkanDevice();

    VkResult result = VK_SUCCESS;

    Vk_Shader* shader = Vk_ShaderManager::GetInstance().GetActiveShader();
    Vk_TextureManager& texture_manager = Vk_TextureManager::GetInstance();

    if (shader == nullptr) {
        LogError(sTag, "Active shader not set");
        return false;
    }

    if (!shader->GetModule(ShaderType::VERTEX) ||
        !shader->GetModule(ShaderType::FRAGMENT)) {
        LogError(sTag, "Coud not get Vertex and/or Fragment shader module");
        return false;
    }

    std::array<VkVertexInputBindingDescription, 1> vertex_binding_descriptions =
        {{
            {
                sVertexBufferBindId,         // binding
                sizeof(Vertex),              // stride
                VK_VERTEX_INPUT_RATE_VERTEX  // inputRate
            },
        }};

    std::array<VkVertexInputAttributeDescription, 4>
        vertex_attribute_descriptions = {{
            {
                0,                           // location
                sVertexBufferBindId,         // binding
                VK_FORMAT_R32G32B32_SFLOAT,  // format
                offsetof(Vertex, position)   // offset
            },
            {
                1,                           // location
                sVertexBufferBindId,         // binding
                VK_FORMAT_R32G32B32_SFLOAT,  // format
                offsetof(Vertex, normal)     // offset
            },
            {
                2,                            // location
                sVertexBufferBindId,          // binding
                VK_FORMAT_R32G32_SFLOAT,      // format
                offsetof(Vertex, tex_coords)  // offset
            },
            {
                3,                              // location
                sVertexBufferBindId,            // binding
                VK_FORMAT_R32G32B32A32_SFLOAT,  // format
                offsetof(Vertex, color)         // offset
            },
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

    std::array<VkPipelineShaderStageCreateInfo, 2> shader_stage_create_infos = {
        {
            // Vertex shader
            {
                VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,  // sType
                nullptr,                                              // pNext
                VkPipelineShaderStageCreateFlags(),                   // flags
                VK_SHADER_STAGE_VERTEX_BIT,                           // stage
                shader->GetModule(ShaderType::VERTEX),                // module
                sShaderEntryPoint,                                    // pName
                nullptr  // pSpecializationInfo
            },
            // Fragment shader
            {
                VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,  // sType
                nullptr,                                              // pNext
                VkPipelineShaderStageCreateFlags(),                   // flags
                VK_SHADER_STAGE_FRAGMENT_BIT,                         // stage
                shader->GetModule(ShaderType::FRAGMENT),              // module
                sShaderEntryPoint,                                    // pName
                nullptr  // pSpecializationInfo
            },
        }};

    VkPipelineInputAssemblyStateCreateInfo input_assembly_state_create_info = {
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,  // sType
        nullptr,                                                      // pNext
        VkPipelineInputAssemblyStateCreateFlags(),                    // flags
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,  // topology
        VK_FALSE                              // primitiveRestartEnable
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
        VK_CULL_MODE_NONE,                // cullMode
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
        VK_FALSE,                             // blendEnable
        VK_BLEND_FACTOR_SRC_ALPHA,            // srcColorBlendFactor
        VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,  // dstColorBlendFactor
        VK_BLEND_OP_ADD,                      // colorBlendOp
        VK_BLEND_FACTOR_SRC_ALPHA,            // srcAlphaBlendFactor
        VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,  // dstAlphaBlendFactor
        VK_BLEND_OP_ADD,                      // alphaBlendOp
        (VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
         VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT)  // colorWriteMask
    };

    VkPipelineDepthStencilStateCreateInfo depth_stencil_info = {
        VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,  // sType
        nullptr,                                                     // pNext
        VkPipelineDepthStencilStateCreateFlags(),                    // flags
        VK_TRUE,                      // depthTestEnable
        VK_TRUE,                      // depthWriteEnable
        VK_COMPARE_OP_LESS_OR_EQUAL,  // depthCompareOp
        VK_FALSE,                     // depthBoundsTestEnable
        VK_FALSE,                     // stencilTestEnable
        {},                           // front
        {},                           // back
        0.0f,                         // minDepthBounds
        1.0f,                         // maxDepthBounds
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
    std::array<VkDynamicState, 2> dynamic_states = {{
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    }};
    VkPipelineDynamicStateCreateInfo dynamic_state_create_info = {
        VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,  // sType
        nullptr,                                               // pNext
        0,                                                     // flags
        static_cast<uint32_t>(dynamic_states.size()),  // dynamicStateCount
        dynamic_states.data()                          // pDynamicStates
    };

    // Create the PipelineLayout
    std::array<VkDescriptorSetLayout, 2> descriptor_set_layouts = {{
        shader->GetUBODescriptorSetLayout(),
        texture_manager.GetDescriptorSetLayout(),
    }};

    VkPipelineLayoutCreateInfo layout_create_info = {
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,         // sType
        nullptr,                                               // pNext
        VkPipelineLayoutCreateFlags(),                         // flags
        static_cast<uint32_t>(descriptor_set_layouts.size()),  // setLayoutCount
        descriptor_set_layouts.data(),                         // pSetLayouts
        0,       // pushConstantRangeCount
        nullptr  // pPushConstantRanges
    };

    result = vkCreatePipelineLayout(device, &layout_create_info, nullptr,
                                    &m_pipeline_layout);
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
        &depth_stencil_info,                // pDepthStencilState
        &color_blend_state_create_info,     // pColorBlendState
        &dynamic_state_create_info,         // pDynamicState
        m_pipeline_layout,                  // layout
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
        vkDestroyPipelineLayout(device, m_pipeline_layout, nullptr);
        m_pipeline_layout = VK_NULL_HANDLE;
        return false;
    }

    return true;
}

bool Vk_RenderWindow::CreateRenderingResources() {
    Vk_Context& context = Vk_Context::GetInstance();

    m_render_resources.resize(m_swapchain.GetImages().size());
    for (size_t i = 0; i < m_render_resources.size(); ++i) {
        if (!AllocateVulkanCommandBuffers(
                context.GetGraphicsQueueCmdPool(), 1,
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

    std::array<VkImageView, 2> attachments = {
        {image_view, m_depth_image.GetView()},
    };

    VkFramebufferCreateInfo framebuffer_create_info = {
        VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,  // sType
        nullptr,                                    // pNext
        VkFramebufferCreateFlags(),                 // flags
        m_render_pass,                              // renderPass
        static_cast<uint32_t>(attachments.size()),  // attachmentCount
        attachments.data(),                         // pAttachments
        static_cast<uint32_t>(m_size.x),            // width
        static_cast<uint32_t>(m_size.y),            // height
        1                                           // layers
    };

    result = vkCreateFramebuffer(device, &framebuffer_create_info, nullptr,
                                 &framebuffer);
    if (result != VK_SUCCESS) {
        LogError(sTag, "Could not create a framebuffer");
        return false;
    }

    return true;
}

bool Vk_RenderWindow::PrepareFrame(VkCommandBuffer command_buffer,
                                   Vk_Image& image,
                                   VkFramebuffer& framebuffer) {
    VkResult result = VK_SUCCESS;

    if (!CreateVulkanFrameBuffer(framebuffer, image.GetView())) {
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

    if (m_present_queue->GetHandle() != m_graphics_queue->GetHandle()) {
        VkImageMemoryBarrier barrier_from_present_to_draw = {
            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,  // sType
            nullptr,                                 // pNext
            VK_ACCESS_MEMORY_READ_BIT,               // srcAccessMask
            VK_ACCESS_MEMORY_READ_BIT,               // dstAccessMask
            VK_IMAGE_LAYOUT_UNDEFINED,               // oldLayout
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,         // newLayout
            m_present_queue->family_index,           // srcQueueFamilyIndex
            m_graphics_queue->family_index,          // dstQueueFamilyIndex
            image.GetHandle(),                       // image
            image_subresource_range                  // subresourceRange
        };
        vkCmdPipelineBarrier(
            command_buffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0,
            nullptr, 1, &barrier_from_present_to_draw);
    }

    std::array<VkClearValue, 2> clear_values = {};
    clear_values[0].color = {{0.0f, 0.0f, 0.0f, 0.0f}};
    clear_values[1].depthStencil = {1.0f, 0};

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
        static_cast<uint32_t>(clear_values.size()),  // clearValueCount
        clear_values.data()                          // pClearValues
    };

    vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info,
                         VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport = {
        0.0f,                          // x
        0.0f,                          // y
        static_cast<float>(m_size.x),  // width
        static_cast<float>(m_size.y),  // height
        0.0f,                          // minDepth
        1.0f                           // maxDepth
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

    vkCmdSetViewport(command_buffer, 0, 1, &viewport);
    vkCmdSetScissor(command_buffer, 0, 1, &scissor);

    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      m_graphics_pipeline);

    Vk_Shader* shader = Vk_ShaderManager::GetInstance().GetActiveShader();

    // Update static uniform buffer
    const Camera* active_camera = GetActiveCamera();

    math::vec3 front_vector;
    math::vec3 light_position;  // TMP: Get this from other
                                //      part a LightManager maybe?
    if (active_camera != nullptr) {
        front_vector = active_camera->GetFrontVector();
        light_position = active_camera->GetPosition();
    }

    UniformBufferObject& ubo = shader->GetUBO();
    ubo.SetAttributeValue("cameraFront", front_vector);
    ubo.SetAttributeValue("lightPosition", light_position);
    ///

    uint32 index = 0;
    while (m_command_work_queue.GetSize() > 0) {
        auto task = m_command_work_queue.Pop();
        task(index++, command_buffer, m_pipeline_layout);
    }

    shader->UploadUniformBuffers();

    vkCmdEndRenderPass(command_buffer);

    if (m_graphics_queue->GetHandle() != m_present_queue->GetHandle()) {
        VkImageMemoryBarrier barrier_from_draw_to_present = {
            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,  // sType
            nullptr,                                 // pNext
            VK_ACCESS_MEMORY_READ_BIT,               // srcAccessMask
            VK_ACCESS_MEMORY_READ_BIT,               // dstAccessMask
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,         // oldLayout
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,         // newLayout
            m_graphics_queue->family_index,          // srcQueueFamilyIndex
            m_present_queue->family_index,           // dstQueueFamilyIndex
            image.GetHandle(),                       // image
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

bool Vk_RenderWindow::CreateDepthResources() {
    Vk_Context& context = Vk_Context::GetInstance();
    PhysicalDeviceParameters& physical_device = context.GetPhysicalDevice();

    auto lFindSupportedFormat = [&physical_device](
                                    const std::vector<VkFormat>& candidates,
                                    VkImageTiling tiling,
                                    VkFormatFeatureFlags features) -> VkFormat {
        for (VkFormat format : candidates) {
            VkFormatProperties properties =
                physical_device.GetFormatProperties(format);
            switch (tiling) {
                case VK_IMAGE_TILING_LINEAR:
                    if ((properties.linearTilingFeatures & features) ==
                        features) {
                        return format;
                    }
                    break;
                case VK_IMAGE_TILING_OPTIMAL:
                    if ((properties.optimalTilingFeatures & features) ==
                        features) {
                        return format;
                    }
                    break;
                default:
                    break;
            }
        }

        return VK_FORMAT_UNDEFINED;
    };

    m_depth_format = lFindSupportedFormat(
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,
         VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

    if (m_depth_format == VK_FORMAT_UNDEFINED) {
        LogError(sTag, "Supported Depth format not found");
        return false;
    }

    m_depth_image.Destroy();

    if (!m_depth_image.CreateImage(
            math::uvec2(m_size), m_depth_format, VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
        LogError(sTag, "Could not create depth image");
        return false;
    }

    if (!m_depth_image.AllocateMemory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
        LogError(sTag, "Could not allocate memory for depth image");
        return false;
    }

    if (!m_depth_image.CreateImageView(m_depth_format,
                                       VK_IMAGE_ASPECT_DEPTH_BIT)) {
        LogError(sTag, "Could not create depth image view");
        return false;
    }

    SubmitGraphicsCommand([this](VkCommandBuffer& command_buffer) {
        bool has_stencil_component =
            (m_depth_format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
             m_depth_format == VK_FORMAT_D24_UNORM_S8_UINT);

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
            VK_QUEUE_FAMILY_IGNORED,    // srcQueueFamilyIndex
            VK_QUEUE_FAMILY_IGNORED,    // dstQueueFamilyIndex
            m_depth_image.GetHandle(),  // image
            {
                aspect_mask,  // aspectMask
                0,            // baseMipLevel
                1,            // levelCount
                0,            // baseArrayLayer
                1             // layerCount
            }                 // subresourceRange
        };

        vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                             VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, 0, 0,
                             nullptr, 0, nullptr, 1, &depth_barrier);
    });

    return true;
}

void Vk_RenderWindow::OnWindowResized(const math::ivec2& size) {
    if (m_surface.GetHandle() != VK_NULL_HANDLE) {
        // Recreate the depth image
        CreateDepthResources();
        // Recreate the Vulkan Swapchain
        m_swapchain.Create(m_surface, size.x, size.y);
        m_command_work_queue.Clear();
    }
}

void Vk_RenderWindow::OnAppWillEnterBackground() {
    RenderWindow::OnAppWillEnterBackground();
    m_swapchain.Destroy();
    m_surface.Destroy();
    m_command_work_queue.Clear();
}

void Vk_RenderWindow::OnAppDidEnterBackground() {
    RenderWindow::OnAppDidEnterBackground();
}

void Vk_RenderWindow::OnAppWillEnterForeground() {
    RenderWindow::OnAppWillEnterForeground();
}

void Vk_RenderWindow::OnAppDidEnterForeground() {
    RenderWindow::OnAppDidEnterForeground();
    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.GetVulkanDevice();

    if (device) {
        vkDeviceWaitIdle(device);

        if (!m_surface.Create(reinterpret_cast<SDL_Window*>(m_window))) {
            LogFatal(sTag, "Could not create the Surface");
            return;
        }

        if (!CheckWSISupport()) {
            PhysicalDeviceParameters& physical_device =
                context.GetPhysicalDevice();
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