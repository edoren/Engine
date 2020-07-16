#include "Vk_Mesh.hpp"

#include <Graphics/3D/Camera.hpp>
#include <Math/Utilities.hpp>
#include <Renderer/RenderStates.hpp>
#include <System/LogManager.hpp>
#include <System/StringFormat.hpp>
#include <System/StringView.hpp>
#include <Util/Container/Vector.hpp>

#include "Vk_Context.hpp"
#include "Vk_Mesh.hpp"
#include "Vk_RenderWindow.hpp"
#include "Vk_Shader.hpp"
#include "Vk_ShaderManager.hpp"
#include "Vk_Texture2D.hpp"
#include "Vk_TextureManager.hpp"

#include <array>
#include <utility>

namespace engine::plugin::vulkan {

namespace {

const StringView sTag("Vk_Mesh");

}  // namespace

Vk_Mesh::Vk_Mesh() = default;

Vk_Mesh::~Vk_Mesh() {
    Vk_Context& context = Vk_Context::GetInstance();
    QueueParameters& graphicsQueue = context.getGraphicsQueue();
    vkQueueWaitIdle(graphicsQueue.getHandle());

    m_vertexBuffer.destroy();
    m_indexBuffer.destroy();
}

void Vk_Mesh::loadFromData(Vector<Vertex> vertices,
                           Vector<uint32> indices,
                           Vector<std::pair<Texture2D*, TextureType>> textures) {
    m_vertices = vertices;
    m_indices = indices;
    m_textures = textures;
    setupMesh();
}

void Vk_Mesh::setupMesh() {
    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.getVulkanDevice();
    QueueParameters& graphicsQueue = context.getGraphicsQueue();

    VkResult result = VK_SUCCESS;

    VkDeviceSize vertexBufferDataSize = sizeof(Vertex) * m_vertices.size();
    VkDeviceSize indexBufferDataSize = sizeof(uint32) * m_indices.size();

    if (!m_vertexBuffer.create(vertexBufferDataSize,
                               (VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT),
                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
        LogError(sTag, "Could not create Vertex Buffer");
        return;
    }

    if (!m_indexBuffer.create(indexBufferDataSize,
                              (VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT),
                              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
        LogError(sTag, "Could not create Index Buffer");
        return;
    }

    Vk_Buffer stagingBuffer;
    if (!stagingBuffer.create(vertexBufferDataSize + indexBufferDataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
        LogError(sTag, "Could not create Staging Buffer");
        return;
    }

    void* stagingBufferMemoryPointer;
    result = vkMapMemory(device, stagingBuffer.getMemory(), 0, stagingBuffer.getSize(), 0, &stagingBufferMemoryPointer);
    if (result != VK_SUCCESS) {
        LogError(sTag, "Could not map memory and upload data to a vertex buffer");
        return;
    }

    byte* vertexStartPosition = reinterpret_cast<byte*>(stagingBufferMemoryPointer);
    byte* indexStartPosition = vertexStartPosition + vertexBufferDataSize;

    std::memcpy(vertexStartPosition, m_vertices.data(), vertexBufferDataSize);

    std::memcpy(indexStartPosition, m_indices.data(), indexBufferDataSize);

    VkMappedMemoryRange flushRange = {
        .sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
        .pNext = nullptr,
        .memory = stagingBuffer.getMemory(),
        .offset = 0,
        .size = VK_WHOLE_SIZE,
    };
    vkFlushMappedMemoryRanges(device, 1, &flushRange);

    vkUnmapMemory(device, stagingBuffer.getMemory());

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

    std::array<VkBufferCopy, 2> bufferCopyInfos = {{
        // Vertex Buffer
        {
            .srcOffset = 0,
            .dstOffset = 0,
            .size = m_vertexBuffer.getSize(),
        },
        // Index Buffer
        {
            .srcOffset = m_vertexBuffer.getSize(),
            .dstOffset = 0,
            .size = m_indexBuffer.getSize(),
        },
    }};

    vkCmdCopyBuffer(commandBuffer, stagingBuffer.getHandle(), m_vertexBuffer.getHandle(), 1, &bufferCopyInfos[0]);

    vkCmdCopyBuffer(commandBuffer, stagingBuffer.getHandle(), m_indexBuffer.getHandle(), 1, &bufferCopyInfos[1]);

    std::array<VkBufferMemoryBarrier, 2> bufferMemoryBarriers = {{
        {
            .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
            .pNext = nullptr,
            .srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT,
            .dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .buffer = m_vertexBuffer.getHandle(),
            .offset = 0,
            .size = VK_WHOLE_SIZE,
        },
        {
            .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
            .pNext = nullptr,
            .srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT,
            .dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .buffer = m_indexBuffer.getHandle(),
            .offset = 0,
            .size = VK_WHOLE_SIZE,
        },
    }};

    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, 0, 0,
                         nullptr, static_cast<uint32_t>(bufferMemoryBarriers.size()), bufferMemoryBarriers.data(), 0,
                         nullptr);

    vkEndCommandBuffer(commandBuffer);

    // Submit command buffer and copy data from staging buffer to the vertex and index buffer
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

    result = vkQueueSubmit(graphicsQueue.getHandle(), 1, &submitInfo, VK_NULL_HANDLE);
    if (result != VK_SUCCESS) {
        LogError(sTag, "Error copying the Mesh data to the Device");
        return;
    }

    vkQueueWaitIdle(graphicsQueue.getHandle());

    stagingBuffer.destroy();
}

void Vk_Mesh::draw(RenderWindow& target, const RenderStates& states) const {
    auto& window = static_cast<Vk_RenderWindow&>(target);

    auto lambda = [this, &window, states](uint32 index, VkCommandBuffer& commandBuffer,
                                          VkPipelineLayout& pipelineLayout) {
        uint32 dynamicOffset = 0;

        Vk_Texture2D* texture = Vk_TextureManager::GetInstance().getActiveTexture2D();
        Vk_Shader* shader = Vk_ShaderManager::GetInstance().getActiveShader();

        for (const auto& pair : m_textures) {
            if (pair.second == TextureType::DIFFUSE) {
                texture = static_cast<Vk_Texture2D*>(pair.first);
            }
        }

        std::array<VkDescriptorSet, 2> descriptorSets;
        size_t arrayPos = 0;

        if (shader) {
            const Camera* activeCamera = window.getActiveCamera();

            math::mat4 modelMatrix = states.transform.getMatrix();
            math::mat4 viewMatrix = (activeCamera != nullptr) ? activeCamera->getViewMatrix() : math::mat4();
            const math::mat4& projectionMatrix = window.getProjectionMatrix();

            math::mat4 mvpMatrix = projectionMatrix * viewMatrix * modelMatrix;
            math::mat4 normalMatrix = modelMatrix.inverse().transpose();

            UniformBufferObject& ubo = shader->getUboDynamic();

            dynamicOffset = index * static_cast<uint32>(ubo.getDynamicAlignment());

            ubo.setAttributeValue("model", modelMatrix, dynamicOffset);
            ubo.setAttributeValue("normalMatrix", normalMatrix, dynamicOffset);
            ubo.setAttributeValue("mvp", mvpMatrix, dynamicOffset);

            descriptorSets[arrayPos++] = shader->getUboDescriptorSet();
        }

        if (texture) {
            descriptorSets[arrayPos++] = texture->getDescriptorSet();
        } else {
            LogFatal(sTag, "Texture not found for Mesh");
        }

        uint32 sVertexBufferBindId = 0;  // TODO: Change where this comes from
        if (m_vertexBuffer.getHandle() != VK_NULL_HANDLE) {
            VkDeviceSize offset = 0;
            vkCmdBindVertexBuffers(commandBuffer, sVertexBufferBindId, 1, &m_vertexBuffer.getHandle(), &offset);
        }
        if (m_indexBuffer.getHandle() != VK_NULL_HANDLE) {
            vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer.getHandle(), 0, VK_INDEX_TYPE_UINT32);
        }

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0,
                                static_cast<uint32>(arrayPos), descriptorSets.data(), 1, &dynamicOffset);

        auto indicesSize = static_cast<uint32>(m_indices.size());
        vkCmdDrawIndexed(commandBuffer, indicesSize, 1, 0, 0, 0);
    };

    window.addCommandExecution(std::move(lambda));
}

}  // namespace engine::plugin::vulkan
