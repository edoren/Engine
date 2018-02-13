#include <System/LogManager.hpp>

#include "Vk_Buffer.hpp"
#include "Vk_Context.hpp"
#include "Vk_Utilities.hpp"

namespace engine {

namespace {

const String sTag("Vk_Buffer");

}  // namespace

Vk_Buffer::Vk_Buffer()
      : m_handle(VK_NULL_HANDLE), m_memory(VK_NULL_HANDLE), m_size(0) {}

Vk_Buffer::~Vk_Buffer() {
    if (m_handle != VK_NULL_HANDLE) {
        Destroy();
    }
}

bool Vk_Buffer::Create(VkDeviceSize size, VkBufferUsageFlags usage,
                       VkMemoryPropertyFlags memory_properties) {
    if (m_handle != VK_NULL_HANDLE) {
        Destroy();
    }

    VkResult result = VK_SUCCESS;

    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.GetVulkanDevice();

    VkBufferCreateInfo buffer_create_info = {
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,  // sType
        nullptr,                               // pNext
        0,                                     // flags
        size,                                  // size
        usage,                                 // usage
        VK_SHARING_MODE_EXCLUSIVE,             // sharingMode
        0,                                     // queueFamilyIndexCount
        nullptr                                // pQueueFamilyIndices
    };

    result = vkCreateBuffer(device, &buffer_create_info, nullptr, &m_handle);
    if (result != VK_SUCCESS) {
        LogError(sTag, "Could not create buffer");
        return false;
    }

    if (!Vk_Utilities::AllocateBufferMemory(m_handle, &m_memory, memory_properties)) {
        LogError(sTag, "Could not allocate memory for a buffer");
        return false;
    }

    if (vkBindBufferMemory(device, m_handle, m_memory, 0) != VK_SUCCESS) {
        LogError(sTag, "Could not bind memory to a buffer");
        return false;
    }

    m_size = size;

    return true;
}

void Vk_Buffer::Destroy() {
    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.GetVulkanDevice();

    if (m_handle) {
        vkDestroyBuffer(device, m_handle, nullptr);
        m_handle = VK_NULL_HANDLE;
    }

    if (m_memory) {
        vkFreeMemory(device, m_memory, nullptr);
        m_memory = VK_NULL_HANDLE;
    }
}

VkBuffer& Vk_Buffer::GetHandle() {
    return m_handle;
}

const VkBuffer& Vk_Buffer::GetHandle() const {
    return m_handle;
}

VkDeviceMemory& Vk_Buffer::GetMemory() {
    return m_memory;
}

VkDeviceSize Vk_Buffer::GetSize() const {
    return m_size;
}

}  // namespace engine
