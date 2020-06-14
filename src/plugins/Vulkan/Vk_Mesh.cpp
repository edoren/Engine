#include "Vk_Mesh.hpp"

#include <Graphics/3D/Camera.hpp>
#include <Math/Utilities.hpp>
#include <Renderer/RenderStates.hpp>
#include <System/LogManager.hpp>
#include <System/StringFormat.hpp>

#include "Vk_Context.hpp"
#include "Vk_Mesh.hpp"
#include "Vk_RenderWindow.hpp"

#include "Vk_Shader.hpp"
#include "Vk_ShaderManager.hpp"
#include "Vk_Texture2D.hpp"
#include "Vk_TextureManager.hpp"

namespace engine {

namespace {

const String sTag("Vk_Mesh");

}  // namespace

Vk_Mesh::Vk_Mesh() {}

Vk_Mesh::~Vk_Mesh() {
    Vk_Context& context = Vk_Context::GetInstance();
    QueueParameters& graphics_queue = context.getGraphicsQueue();
    vkQueueWaitIdle(graphics_queue.getHandle());

    m_vertexBuffer.destroy();
    m_indexBuffer.destroy();
}

void Vk_Mesh::loadFromData(std::vector<Vertex> vertices,
                           std::vector<uint32> indices,
                           std::vector<std::pair<Texture2D*, TextureType>> textures) {
    m_vertices = vertices;
    m_indices = indices;
    m_textures = textures;
    setupMesh();
}

void Vk_Mesh::setupMesh() {
    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.getVulkanDevice();
    QueueParameters& graphics_queue = context.getGraphicsQueue();

    VkResult result = VK_SUCCESS;

    VkDeviceSize vertex_buffer_data_size = sizeof(Vertex) * m_vertices.size();
    VkDeviceSize index_buffer_data_size = sizeof(uint32) * m_indices.size();

    if (!m_vertexBuffer.create(vertex_buffer_data_size,
                               (VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT),
                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
        LogError(sTag, "Could not create Vertex Buffer");
        return;
    }

    if (!m_indexBuffer.create(index_buffer_data_size,
                              (VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT),
                              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
        LogError(sTag, "Could not create Index Buffer");
        return;
    }

    Vk_Buffer staging_buffer;
    if (!staging_buffer.create(vertex_buffer_data_size + index_buffer_data_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
        LogError(sTag, "Could not create Staging Buffer");
        return;
    }

    void* staging_buffer_memory_pointer;
    result =
        vkMapMemory(device, staging_buffer.getMemory(), 0, staging_buffer.getSize(), 0, &staging_buffer_memory_pointer);
    if (result != VK_SUCCESS) {
        LogError(sTag, "Could not map memory and upload data to a vertex buffer");
        return;
    }

    byte* vertex_start_position = reinterpret_cast<byte*>(staging_buffer_memory_pointer);
    byte* index_start_position = vertex_start_position + vertex_buffer_data_size;

    std::memcpy(vertex_start_position, m_vertices.data(), vertex_buffer_data_size);

    std::memcpy(index_start_position, m_indices.data(), index_buffer_data_size);

    VkMappedMemoryRange flush_range = {
        VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,  // sType
        nullptr,                                // pNext
        staging_buffer.getMemory(),             // memory
        0,                                      // offset
        VK_WHOLE_SIZE                           // size
    };
    vkFlushMappedMemoryRanges(device, 1, &flush_range);

    vkUnmapMemory(device, staging_buffer.getMemory());

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

    std::array<VkBufferCopy, 2> buffer_copy_infos = {{
        // Vertex Buffer
        {
            0,                        // srcOffset
            0,                        // dstOffset
            m_vertexBuffer.getSize()  // size
        },
        // Index Buffer
        {
            m_vertexBuffer.getSize(),  // srcOffset
            0,                         // dstOffset
            m_indexBuffer.getSize()    // size
        },
    }};

    vkCmdCopyBuffer(command_buffer, staging_buffer.getHandle(), m_vertexBuffer.getHandle(), 1, &buffer_copy_infos[0]);

    vkCmdCopyBuffer(command_buffer, staging_buffer.getHandle(), m_indexBuffer.getHandle(), 1, &buffer_copy_infos[1]);

    std::array<VkBufferMemoryBarrier, 2> buffer_memory_barriers = {{
        {
            VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,  // sType;
            nullptr,                                  // pNext
            VK_ACCESS_MEMORY_WRITE_BIT,               // srcAccessMask
            VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,      // dstAccessMask
            VK_QUEUE_FAMILY_IGNORED,                  // srcQueueFamilyIndex
            VK_QUEUE_FAMILY_IGNORED,                  // dstQueueFamilyIndex
            m_vertexBuffer.getHandle(),               // buffer
            0,                                        // offset
            VK_WHOLE_SIZE                             // size
        },
        {
            VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,  // sType;
            nullptr,                                  // pNext
            VK_ACCESS_MEMORY_WRITE_BIT,               // srcAccessMask
            VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,      // dstAccessMask
            VK_QUEUE_FAMILY_IGNORED,                  // srcQueueFamilyIndex
            VK_QUEUE_FAMILY_IGNORED,                  // dstQueueFamilyIndex
            m_indexBuffer.getHandle(),                // buffer
            0,                                        // offset
            VK_WHOLE_SIZE                             // size
        },
    }};

    vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, 0, 0,
                         nullptr, static_cast<uint32_t>(buffer_memory_barriers.size()), buffer_memory_barriers.data(),
                         0, nullptr);

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

    result = vkQueueSubmit(graphics_queue.getHandle(), 1, &submit_info, VK_NULL_HANDLE);
    if (result != VK_SUCCESS) {
        LogError(sTag, "Error copying the Mesh data to the Device");
        return;
    }

    vkQueueWaitIdle(graphics_queue.getHandle());

    staging_buffer.destroy();
}

void Vk_Mesh::draw(RenderWindow& target, const RenderStates& states) const {
    Vk_RenderWindow& window = static_cast<Vk_RenderWindow&>(target);

    auto lambda = [this, &window, states](uint32 index, VkCommandBuffer& command_buffer,
                                          VkPipelineLayout& pipeline_layout) {
        uint32 dynamic_offset = 0;

        Vk_Texture2D* texture = Vk_TextureManager::GetInstance().getActiveTexture2D();
        Vk_Shader* shader = Vk_ShaderManager::GetInstance().getActiveShader();

        for (const auto& pair : m_textures) {
            if (pair.second == TextureType::DIFFUSE) {
                texture = static_cast<Vk_Texture2D*>(pair.first);
            }
        }

        std::array<VkDescriptorSet, 2> descriptor_sets;
        size_t array_pos = 0;

        if (shader) {
            const Camera* active_camera = window.getActiveCamera();

            math::mat4 model_matrix = states.transform.getMatrix();
            math::mat4 view_matrix = (active_camera != nullptr) ? active_camera->getViewMatrix() : math::mat4();
            const math::mat4& projection_matrix = window.getProjectionMatrix();

            math::mat4 mvp_matrix = projection_matrix * view_matrix * model_matrix;
            math::mat4 normal_matrix = model_matrix.inverse().transpose();

            UniformBufferObject& ubo = shader->getUboDynamic();

            dynamic_offset = index * static_cast<uint32>(ubo.getDynamicAlignment());

            ubo.setAttributeValue("model", model_matrix, dynamic_offset);
            ubo.setAttributeValue("normalMatrix", normal_matrix, dynamic_offset);
            ubo.setAttributeValue("mvp", mvp_matrix, dynamic_offset);

            descriptor_sets[array_pos++] = shader->getUboDescriptorSet();
        }

        if (texture) {
            descriptor_sets[array_pos++] = texture->getDescriptorSet();
        } else {
            LogFatal(sTag, "Texture not found for Mesh");
        }

        uint32 sVertexBufferBindId = 0;  // TODO: Change where this comes from
        if (m_vertexBuffer.getHandle() != VK_NULL_HANDLE) {
            VkDeviceSize offset = 0;
            vkCmdBindVertexBuffers(command_buffer, sVertexBufferBindId, 1, &m_vertexBuffer.getHandle(), &offset);
        }
        if (m_indexBuffer.getHandle() != VK_NULL_HANDLE) {
            vkCmdBindIndexBuffer(command_buffer, m_indexBuffer.getHandle(), 0, VK_INDEX_TYPE_UINT32);
        }

        vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0,
                                static_cast<uint32>(array_pos), descriptor_sets.data(), 1, &dynamic_offset);

        uint32 indices_size = static_cast<uint32>(m_indices.size());
        vkCmdDrawIndexed(command_buffer, indices_size, 1, 0, 0, 0);
    };

    window.addCommandExecution(std::move(lambda));
}

}  // namespace engine
