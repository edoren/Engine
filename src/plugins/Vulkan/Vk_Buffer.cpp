#include <System/LogManager.hpp>
#include <System/StringView.hpp>

#include "Vk_Buffer.hpp"
#include "Vk_Context.hpp"
#include "Vk_Utilities.hpp"

namespace engine::plugin::vulkan {

namespace {

const StringView sTag("Vk_Buffer");

}  // namespace

Vk_Buffer::Vk_Buffer() : m_handle(VK_NULL_HANDLE), m_memory(VK_NULL_HANDLE), m_size(0) {}

Vk_Buffer::~Vk_Buffer() {
    if (m_handle != VK_NULL_HANDLE) {
        destroy();
    }
}

bool Vk_Buffer::create(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryProperties) {
    if (m_handle != VK_NULL_HANDLE) {
        destroy();
    }

    VkResult result = VK_SUCCESS;

    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.getVulkanDevice();

    VkBufferCreateInfo bufferCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .size = size,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,
    };

    result = vkCreateBuffer(device, &bufferCreateInfo, nullptr, &m_handle);
    if (result != VK_SUCCESS) {
        LogError(sTag, "Could not create buffer");
        return false;
    }

    if (!Vk_Utilities::AllocateBufferMemory(m_handle, &m_memory, memoryProperties)) {
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

void Vk_Buffer::destroy() {
    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.getVulkanDevice();

    if (m_handle) {
        vkDestroyBuffer(device, m_handle, nullptr);
        m_handle = VK_NULL_HANDLE;
    }

    if (m_memory) {
        vkFreeMemory(device, m_memory, nullptr);
        m_memory = VK_NULL_HANDLE;
    }
}

VkBuffer& Vk_Buffer::getHandle() {
    return m_handle;
}

const VkBuffer& Vk_Buffer::getHandle() const {
    return m_handle;
}

VkDeviceMemory& Vk_Buffer::getMemory() {
    return m_memory;
}

VkDeviceSize Vk_Buffer::getSize() const {
    return m_size;
}

}  // namespace engine::plugin::vulkan
