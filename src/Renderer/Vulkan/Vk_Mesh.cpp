#include "Vk_Mesh.hpp"

#include <Math/Utilities.hpp>
#include <System/LogManager.hpp>
#include <System/StringFormat.hpp>

#include "Vk_Context.hpp"
#include "Vk_Mesh.hpp"
#include "Vk_RenderWindow.hpp"

namespace engine {

namespace {

const String sTag("Vk_Mesh");

}  // namespace

Vk_Mesh::Vk_Mesh() : m_vertex_buffer(), m_index_buffer() {}

Vk_Mesh::~Vk_Mesh() {
    Vk_Context& context = Vk_Context::GetInstance();
    QueueParameters& graphics_queue = context.GetGraphicsQueue();
    vkQueueWaitIdle(graphics_queue.GetHandle());

    m_vertex_buffer.Destroy();
    m_index_buffer.Destroy();
}

void Vk_Mesh::LoadFromData(
    std::vector<Vertex> vertices, std::vector<uint32> indices,
    std::vector<std::pair<Texture2D*, TextureType>> textures) {
    m_vertices = vertices;
    m_indices = indices;
    m_textures = textures;
    SetupMesh();
}

void Vk_Mesh::SetupMesh() {
    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.GetVulkanDevice();
    QueueParameters& graphics_queue = context.GetGraphicsQueue();

    VkResult result = VK_SUCCESS;

    VkDeviceSize vertex_buffer_data_size = sizeof(Vertex) * m_vertices.size();
    VkDeviceSize index_buffer_data_size = sizeof(uint32) * m_indices.size();

    if (!m_vertex_buffer.Create(vertex_buffer_data_size,
                                (VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                                 VK_BUFFER_USAGE_TRANSFER_DST_BIT),
                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
        LogError(sTag, "Could not create Vertex Buffer");
        return;
    }

    if (!m_index_buffer.Create(index_buffer_data_size,
                               (VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
                                VK_BUFFER_USAGE_TRANSFER_DST_BIT),
                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
        LogError(sTag, "Could not create Index Buffer");
        return;
    }

    Vk_Buffer staging_buffer;
    if (!staging_buffer.Create(vertex_buffer_data_size + index_buffer_data_size,
                               VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
        LogError(sTag, "Could not create Staging Buffer");
        return;
    }

    void* staging_buffer_memory_pointer;
    result = vkMapMemory(device, staging_buffer.GetMemory(), 0,
                         staging_buffer.GetSize(), 0,
                         &staging_buffer_memory_pointer);
    if (result != VK_SUCCESS) {
        LogError(sTag,
                 "Could not map memory and upload data to a vertex buffer");
        return;
    }

    byte* vertex_start_position =
        reinterpret_cast<byte*>(staging_buffer_memory_pointer);
    byte* index_start_position =
        vertex_start_position + vertex_buffer_data_size;

    std::memcpy(vertex_start_position, m_vertices.data(),
                vertex_buffer_data_size);

    std::memcpy(index_start_position, m_indices.data(), index_buffer_data_size);

    VkMappedMemoryRange flush_range = {
        VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,  // sType
        nullptr,                                // pNext
        staging_buffer.GetMemory(),             // memory
        0,                                      // offset
        m_vertex_buffer.GetSize()               // size
    };
    vkFlushMappedMemoryRanges(device, 1, &flush_range);

    vkUnmapMemory(device, staging_buffer.GetMemory());

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

    std::array<VkBufferCopy, 2> buffer_copy_infos = {{
        // Vertex Buffer
        {
            0,                         // srcOffset
            0,                         // dstOffset
            m_vertex_buffer.GetSize()  // size
        },
        // Index Buffer
        {
            m_vertex_buffer.GetSize(),  // srcOffset
            0,                          // dstOffset
            m_index_buffer.GetSize()    // size
        },
    }};

    vkCmdCopyBuffer(command_buffer, staging_buffer.GetHandle(),
                    m_vertex_buffer.GetHandle(), 1, &buffer_copy_infos[0]);

    vkCmdCopyBuffer(command_buffer, staging_buffer.GetHandle(),
                    m_index_buffer.GetHandle(), 1, &buffer_copy_infos[1]);

    std::array<VkBufferMemoryBarrier, 2> buffer_memory_barriers = {{
        {
            VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,  // sType;
            nullptr,                                  // pNext
            VK_ACCESS_MEMORY_WRITE_BIT,               // srcAccessMask
            VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,      // dstAccessMask
            VK_QUEUE_FAMILY_IGNORED,                  // srcQueueFamilyIndex
            VK_QUEUE_FAMILY_IGNORED,                  // dstQueueFamilyIndex
            m_vertex_buffer.GetHandle(),              // buffer
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
            m_index_buffer.GetHandle(),               // buffer
            0,                                        // offset
            VK_WHOLE_SIZE                             // size
        },
    }};

    vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, 0, 0, nullptr,
                         static_cast<uint32_t>(buffer_memory_barriers.size()),
                         buffer_memory_barriers.data(), 0, nullptr);

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

    result = vkQueueSubmit(graphics_queue.GetHandle(), 1, &submit_info,
                           VK_NULL_HANDLE);
    if (result != VK_SUCCESS) {
        LogError(sTag, "Error copying the Mesh data to the Device");
        return;
    }

    vkQueueWaitIdle(graphics_queue.GetHandle());

    staging_buffer.Destroy();
}

void Vk_Mesh::Draw(RenderWindow& target) const {
    Vk_RenderWindow* window = static_cast<Vk_RenderWindow*>(&target);

    // for (auto& texture : m_textures) {
    //     switch (texture.second) {
    //         case TextureType::eDiffuse:
    //             break;
    //         case TextureType::eSpecular:
    //         default:
    //             break;
    //     }

    //     Vk_Texture2D* curr_texture =
    //         reinterpret_cast<Vk_Texture2D*>(texture.first);
    //     if (curr_texture) {
    //         curr_texture->Use();
    //     }
    // }

    auto lambda = [this](VkCommandBuffer& command_buffer) {
        if (m_vertex_buffer.GetHandle() != VK_NULL_HANDLE) {
            VkDeviceSize offset = 0;
            vkCmdBindVertexBuffers(command_buffer, 0, 1,
                                   &m_vertex_buffer.GetHandle(), &offset);
        }
        if (m_index_buffer.GetHandle() != VK_NULL_HANDLE) {
            vkCmdBindIndexBuffer(command_buffer, m_index_buffer.GetHandle(), 0,
                                 VK_INDEX_TYPE_UINT32);
        }

        uint32 indices_size = static_cast<uint32>(m_indices.size());
        vkCmdDrawIndexed(command_buffer, indices_size, 1, 0, 0, 0);
    };

    window->AddCommandExecution(std::move(lambda));
}

}  // namespace engine
