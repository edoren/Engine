#include <Graphics/3D/Camera.hpp>
#include <Renderer/UniformBufferObject.hpp>
#include <Renderer/Vertex.hpp>
#include <System/FileSystem.hpp>
#include <System/LogManager.hpp>
#include <System/StringFormat.hpp>
#include <System/StringView.hpp>
#include <Util/Container/Vector.hpp>

#include "Vk_RenderWindow.hpp"
#include "Vk_Shader.hpp"
#include "Vk_ShaderManager.hpp"
#include "Vk_Texture2D.hpp"
#include "Vk_TextureManager.hpp"

#include <array>

namespace engine::plugin::vulkan {

namespace {

const StringView sTag("Vk_RenderWindow");

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
    math::ivec2 initialPos(SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    uint32 windowFlags(SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

    m_window = SDL_CreateWindow(name.getData(), initialPos.x, initialPos.y, size.x, size.y, windowFlags);
    if (!m_window) {
        LogError(sTag, "SDL_CreateWindow fail: {}", SDL_GetError());
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
        PhysicalDeviceParameters& physicalDevice = context.getPhysicalDevice();
        LogError(sTag, "Physical device {} doesn't include WSI support", physicalDevice.properties.deviceName);
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
    for (Vk_RenderResource& renderResource : m_renderResources) {
        if (!renderResource.create()) {
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

    Vk_RenderResource& currentRenderingResource = m_renderResources[sResourceIndex];
    uint32_t imageIndex;

    sResourceIndex = (sResourceIndex + 1) % m_swapchain.getImages().size();

    result = vkWaitForFences(device, 1, &currentRenderingResource.fence, VK_FALSE, 1000000000);
    if (result == VK_TIMEOUT) {
        LogError(sTag, "Waiting for fence takes too long");
        return;
    }

    vkResetFences(device, 1, &currentRenderingResource.fence);

    result = vkAcquireNextImageKHR(device, m_swapchain.getHandle(), UINT64_MAX,
                                   currentRenderingResource.imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
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

    if (!prepareFrame(currentRenderingResource.commandBuffer, m_swapchain.getImages()[imageIndex],
                      currentRenderingResource.framebuffer)) {
        return;
    }

    VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = nullptr,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &currentRenderingResource.imageAvailableSemaphore,
        .pWaitDstStageMask = &waitDstStageMask,
        .commandBufferCount = 1,
        .pCommandBuffers = &currentRenderingResource.commandBuffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &currentRenderingResource.finishedRenderingSemaphore,
    };

    result = vkQueueSubmit(m_graphicsQueue->getHandle(), 1, &submitInfo, currentRenderingResource.fence);

    if (result != VK_SUCCESS) {
        LogError(sTag, "Error submitting the command buffers");
        return;
    }

    VkPresentInfoKHR presentInfo = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext = nullptr,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &currentRenderingResource.finishedRenderingSemaphore,
        .swapchainCount = 1,
        .pSwapchains = &m_swapchain.getHandle(),
        .pImageIndices = &imageIndex,
        .pResults = nullptr,
    };

    result = vkQueuePresentKHR(m_presentQueue->getHandle(), &presentInfo);

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
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = context.getGraphicsQueueCmdPool(),
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };

    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
    result = vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);
    if (result != VK_SUCCESS || commandBuffer == VK_NULL_HANDLE) {
        LogError(sTag, "Could not allocate command buffer");
        return;
    }

    VkCommandBufferBeginInfo commandBufferBeginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = nullptr,
    };

    vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);

    // Call the function and pass the command buffer
    func(commandBuffer);

    vkEndCommandBuffer(commandBuffer);

    // Submit command buffer and copy data from staging buffer to the vertex
    // and index buffer
    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = nullptr,
        .waitSemaphoreCount = 0,
        .pWaitSemaphores = nullptr,
        .pWaitDstStageMask = nullptr,
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer,
        .signalSemaphoreCount = 0,
        .pSignalSemaphores = nullptr,
    };

    result = vkQueueSubmit(context.getGraphicsQueue().getHandle(), 1, &submitInfo, VK_NULL_HANDLE);
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
    VkBool32 wsiSupport;
    Vk_Context& context = Vk_Context::GetInstance();
    PhysicalDeviceParameters& physicalDevice = context.getPhysicalDevice();
    QueueParameters& graphicsQueue = context.getGraphicsQueue();

    vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice.getHandle(), graphicsQueue.familyIndex, m_surface.getHandle(),
                                         &wsiSupport);
    return wsiSupport == VK_TRUE;
}

bool Vk_RenderWindow::createVulkanRenderPass() {
    VkResult result = VK_SUCCESS;

    // Create the attachment descriptions
    std::array<VkAttachmentDescription, 2> attachmentDescriptions = {{
        {
            .flags = VkAttachmentDescriptionFlags(),
            .format = m_swapchain.getFormat(),
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        },
        {
            .flags = VkAttachmentDescriptionFlags(),
            .format = m_depthFormat,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        },
    }};

    std::array<VkAttachmentReference, 2> attachmentReferences = {{
        {
            .attachment = 0,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        },
        {
            .attachment = 1,
            .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        },
    }};

    std::array<VkSubpassDescription, 1> subpassDescriptions = {{
        {
            .flags = VkSubpassDescriptionFlags(),
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .inputAttachmentCount = 0,
            .pInputAttachments = nullptr,
            .colorAttachmentCount = 1,
            .pColorAttachments = &attachmentReferences[0],
            .pResolveAttachments = nullptr,
            .pDepthStencilAttachment = &attachmentReferences[1],
            .preserveAttachmentCount = 0,
            .pPreserveAttachments = nullptr,
        },
    }};

    std::array<VkSubpassDependency, 2> dependencies = {{
        {
            .srcSubpass = VK_SUBPASS_EXTERNAL,
            .dstSubpass = 0,
            .srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .srcAccessMask = VK_ACCESS_MEMORY_READ_BIT,
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
        },
        {
            .srcSubpass = 0,
            .dstSubpass = VK_SUBPASS_EXTERNAL,
            .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .dstAccessMask = VK_ACCESS_MEMORY_READ_BIT,
            .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
        },
    }};

    VkRenderPassCreateInfo renderPassCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pNext = nullptr,
        .flags = VkRenderPassCreateFlags(),
        .attachmentCount = static_cast<uint32_t>(attachmentDescriptions.size()),
        .pAttachments = attachmentDescriptions.data(),
        .subpassCount = static_cast<uint32_t>(subpassDescriptions.size()),
        .pSubpasses = subpassDescriptions.data(),
        .dependencyCount = static_cast<uint32_t>(dependencies.size()),
        .pDependencies = dependencies.data(),
    };

    // NOTES: Dependencies are important for performance

    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.getVulkanDevice();
    result = vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &m_renderPass);
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
    Vk_TextureManager& textureManager = Vk_TextureManager::GetInstance();

    if (shader == nullptr) {
        LogError(sTag, "Active shader not set");
        return false;
    }

    if (!shader->getModule(ShaderType::VERTEX) || !shader->getModule(ShaderType::FRAGMENT)) {
        LogError(sTag, "Coud not get Vertex and/or Fragment shader module");
        return false;
    }

    std::array<VkVertexInputBindingDescription, 1> vertexBindingDescriptions = {{
        {
            .binding = sVertexBufferBindId,
            .stride = sizeof(Vertex),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
        },
    }};

    const Vk_VertexLayout& vertexLayout = shader->getVertexLayout();

    auto vertexInputAttribDescription = vertexLayout.getVertexInputAttributeDescription(sVertexBufferBindId);

    VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .vertexBindingDescriptionCount = static_cast<uint32_t>(vertexBindingDescriptions.size()),
        .pVertexBindingDescriptions = vertexBindingDescriptions.data(),
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttribDescription.size()),
        .pVertexAttributeDescriptions = vertexInputAttribDescription.data(),
    };

    std::array<VkPipelineShaderStageCreateInfo, 2> shaderStageCreateInfos = {{
        // Vertex shader
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .pNext = nullptr,
            .flags = VkPipelineShaderStageCreateFlags(),
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = shader->getModule(ShaderType::VERTEX),
            .pName = sShaderEntryPoint,
            .pSpecializationInfo = nullptr,
        },
        // Fragment shader
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .pNext = nullptr,
            .flags = VkPipelineShaderStageCreateFlags(),
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = shader->getModule(ShaderType::FRAGMENT),
            .pName = sShaderEntryPoint,
            .pSpecializationInfo = nullptr,
        },
    }};

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = VkPipelineInputAssemblyStateCreateFlags(),
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE,
    };

    VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = VkPipelineViewportStateCreateFlags(),
        .viewportCount = 1,
        .pViewports = nullptr,
        .scissorCount = 1,
        .pScissors = nullptr,
    };

    VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = VkPipelineRasterizationStateCreateFlags(),
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_NONE,
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .depthBiasConstantFactor = 0.0F,
        .depthBiasClamp = 0.0F,
        .depthBiasSlopeFactor = 0.0F,
        .lineWidth = 1.0F,
    };

    VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = VkPipelineMultisampleStateCreateFlags(),
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE,
        .minSampleShading = 1.0F,
        .pSampleMask = nullptr,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE,
    };

    VkPipelineColorBlendAttachmentState colorBlendAttachmentState = {
        .blendEnable = VK_FALSE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .alphaBlendOp = VK_BLEND_OP_ADD,
        .colorWriteMask = (VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                           VK_COLOR_COMPONENT_A_BIT)  // RGBA
    };

    VkPipelineDepthStencilStateCreateInfo depthStencilInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = VkPipelineDepthStencilStateCreateFlags(),
        .depthTestEnable = VK_TRUE,
        .depthWriteEnable = VK_TRUE,
        .depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL,
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable = VK_FALSE,
        .front = {},
        .back = {},
        .minDepthBounds = 0.0F,
        .maxDepthBounds = 1.0F,
    };

    VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = VkPipelineColorBlendStateCreateFlags(),
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachmentState,
        .blendConstants = {0.0F, 0.0F, 0.0F, 0.0F},
    };

    // Define the pipeline dynamic states
    std::array<VkDynamicState, 2> dynamicStates = {{
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    }};
    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
        .pDynamicStates = dynamicStates.data(),
    };

    // Create the PipelineLayout
    std::array<VkDescriptorSetLayout, 2> descriptorSetLayouts = {{
        shader->getUboDescriptorSetLayout(),
        textureManager.getDescriptorSetLayout(),
    }};

    VkPipelineLayoutCreateInfo layoutCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = VkPipelineLayoutCreateFlags(),
        .setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size()),
        .pSetLayouts = descriptorSetLayouts.data(),
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = nullptr,
    };

    result = vkCreatePipelineLayout(device, &layoutCreateInfo, nullptr, &m_pipelineLayout);
    if (result != VK_SUCCESS) {
        LogError(sTag, "Could not create pipeline layout");
        return false;
    }

    VkGraphicsPipelineCreateInfo pipelineCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = nullptr,
        .flags = VkPipelineCreateFlags(),
        .stageCount = static_cast<uint32_t>(shaderStageCreateInfos.size()),
        .pStages = shaderStageCreateInfos.data(),
        .pVertexInputState = &vertexInputStateCreateInfo,
        .pInputAssemblyState = &inputAssemblyStateCreateInfo,
        .pTessellationState = nullptr,
        .pViewportState = &viewportStateCreateInfo,
        .pRasterizationState = &rasterizationStateCreateInfo,
        .pMultisampleState = &multisampleStateCreateInfo,
        .pDepthStencilState = &depthStencilInfo,
        .pColorBlendState = &colorBlendStateCreateInfo,
        .pDynamicState = &dynamicStateCreateInfo,
        .layout = m_pipelineLayout,
        .renderPass = m_renderPass,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = -1,
    };

    result = vkCreateGraphicsPipelines(device, VkPipelineCache(), 1, &pipelineCreateInfo, nullptr, &m_graphicsPipeline);
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

    VkFramebufferCreateInfo framebufferCreateInfo = {.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                                                     .pNext = nullptr,
                                                     .flags = VkFramebufferCreateFlags(),
                                                     .renderPass = m_renderPass,
                                                     .attachmentCount = static_cast<uint32_t>(attachments.size()),
                                                     .pAttachments = attachments.data(),
                                                     .width = static_cast<uint32_t>(m_size.x),
                                                     .height = static_cast<uint32_t>(m_size.y),
                                                     .layers = 1};

    result = vkCreateFramebuffer(device, &framebufferCreateInfo, nullptr, &framebuffer);
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

    VkCommandBufferBeginInfo commandBufferBeginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = nullptr,
    };

    vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);

    VkImageSubresourceRange imageSubresourceRange = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1,
    };

    if (m_presentQueue->getHandle() != m_graphicsQueue->getHandle()) {
        VkImageMemoryBarrier barrierFromPresentToDraw = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .pNext = nullptr,
            .srcAccessMask = VK_ACCESS_MEMORY_READ_BIT,
            .dstAccessMask = VK_ACCESS_MEMORY_READ_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            .srcQueueFamilyIndex = m_presentQueue->familyIndex,
            .dstQueueFamilyIndex = m_graphicsQueue->familyIndex,
            .image = image.getHandle(),
            .subresourceRange = imageSubresourceRange,
        };
        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                             VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1,
                             &barrierFromPresentToDraw);
    }

    std::array<VkClearValue, 2> clearValues = {};
    clearValues[0].color = {{0.0F, 0.0F, 0.0F, 0.0F}};
    clearValues[1].depthStencil = {1.0F, 0};

    VkRenderPassBeginInfo renderPassBeginInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .pNext = nullptr,
        .renderPass = m_renderPass,
        .framebuffer = framebuffer,
        .renderArea =
            {
                .offset =
                    {
                        .x = 0,
                        .y = 0,
                    },
                .extent =
                    {
                        .width = static_cast<uint32_t>(m_size.x),
                        .height = static_cast<uint32_t>(m_size.y),
                    },
            },
        .clearValueCount = static_cast<uint32_t>(clearValues.size()),
        .pClearValues = clearValues.data(),
    };

    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport = {
        .x = 0.0F,
        .y = 0.0F,
        .width = static_cast<float>(m_size.x),
        .height = static_cast<float>(m_size.y),
        .minDepth = 0.0F,
        .maxDepth = 1.0F,
    };

    VkRect2D scissor = {
        .offset =
            {
                .x = 0,
                .y = 0,
            },
        .extent =
            {
                .width = static_cast<uint32_t>(m_size.x),
                .height = static_cast<uint32_t>(m_size.y),
            },
    };

    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);

    Vk_Shader* shader = Vk_ShaderManager::GetInstance().getActiveShader();

    // Update static uniform buffer
    const Camera* activeCamera = getActiveCamera();

    math::vec3 frontVector;
    math::vec3 lightPosition;  // TMP: Get this from other
                               //      part a LightManager maybe?
    if (activeCamera != nullptr) {
        frontVector = activeCamera->getFrontVector();
        lightPosition = activeCamera->getPosition();
    }

    UniformBufferObject& ubo = shader->getUbo();
    ubo.setAttributeValue("cameraFront", frontVector);
    ubo.setAttributeValue("lightPosition", lightPosition);
    ///

    uint32 index = 0;
    while (m_commandWorkQueue.getSize() > 0) {
        auto task = m_commandWorkQueue.pop();
        task(index++, commandBuffer, m_pipelineLayout);
    }

    shader->uploadUniformBuffers();

    vkCmdEndRenderPass(commandBuffer);

    if (m_graphicsQueue->getHandle() != m_presentQueue->getHandle()) {
        VkImageMemoryBarrier barrierFromDrawToPresent = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .pNext = nullptr,
            .srcAccessMask = VK_ACCESS_MEMORY_READ_BIT,
            .dstAccessMask = VK_ACCESS_MEMORY_READ_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            .srcQueueFamilyIndex = m_graphicsQueue->familyIndex,
            .dstQueueFamilyIndex = m_presentQueue->familyIndex,
            .image = image.getHandle(),
            .subresourceRange = imageSubresourceRange,
        };
        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                             VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1,
                             &barrierFromDrawToPresent);
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
    PhysicalDeviceParameters& physicalDevice = context.getPhysicalDevice();

    auto lFindSupportedFormat = [&physicalDevice](const Vector<VkFormat>& candidates, VkImageTiling tiling,
                                                  VkFormatFeatureFlags features) -> VkFormat {
        for (VkFormat format : candidates) {
            VkFormatProperties properties = physicalDevice.getFormatProperties(format);
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
        bool hasStencilComponent =
            (m_depthFormat == VK_FORMAT_D32_SFLOAT_S8_UINT || m_depthFormat == VK_FORMAT_D24_UNORM_S8_UINT);

        VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        if (hasStencilComponent) {
            aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }

        VkImageMemoryBarrier depthBarrier = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .pNext = nullptr,
            .srcAccessMask = 0,
            .dstAccessMask =
                (VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT),
            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = m_depthImage.getHandle(),
            .subresourceRange =
                {
                    .aspectMask = aspectMask,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                },
        };

        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                             VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, 0, 0, nullptr, 0, nullptr, 1, &depthBarrier);

        LogInfo(sTag, "Depth resources created with dimensions [{}, {}]", m_size.x, m_size.y);
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
            PhysicalDeviceParameters& physicalDevice = context.getPhysicalDevice();
            LogFatal(sTag, "Physical device {} doesn't include WSI support", physicalDevice.properties.deviceName);
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
        for (Vk_RenderResource& renderResource : m_renderResources) {
            if (!renderResource.create()) {
                LogFatal(sTag, "Could not create the RenderingResources");
                return;
            }
        }
    }
}

}  // namespace engine::plugin::vulkan
