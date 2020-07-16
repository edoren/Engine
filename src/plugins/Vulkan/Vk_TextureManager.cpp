#include <System/LogManager.hpp>
#include <System/StringView.hpp>
#include <Util/Container/Vector.hpp>

#include "Vk_Context.hpp"
#include "Vk_Texture2D.hpp"
#include "Vk_TextureManager.hpp"

#include <memory>

namespace engine::plugin::vulkan {

namespace {

const StringView sTag("Vk_TextureManager");

const uint32 sMaxDescriptorSets(256);

Vk_TextureManager* sDerivedInstance = nullptr;

}  // namespace

Vk_TextureManager& Vk_TextureManager::GetInstance() {
    assert(sDerivedInstance);
    return (*sDerivedInstance);
}

Vk_TextureManager* Vk_TextureManager::GetInstancePtr() {
    return sDerivedInstance;
}

Vk_TextureManager::Vk_TextureManager() : m_descriptorPool(VK_NULL_HANDLE), m_descriptorSetLayout(VK_NULL_HANDLE) {
    TextureManager& baseInstance = TextureManager::GetInstance();
    sDerivedInstance = reinterpret_cast<Vk_TextureManager*>(&baseInstance);

    createDescriptorPool();
    createDescriptorSetLayout();
}

Vk_TextureManager::~Vk_TextureManager() {
    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.getVulkanDevice();

    if (m_descriptorPool != VK_NULL_HANDLE) {
        vkDestroyDescriptorPool(device, m_descriptorPool, nullptr);
        m_descriptorPool = VK_NULL_HANDLE;
    }

    if (m_descriptorSetLayout != VK_NULL_HANDLE) {
        vkDestroyDescriptorSetLayout(device, m_descriptorSetLayout, nullptr);
        m_descriptorSetLayout = VK_NULL_HANDLE;
    }

    sDerivedInstance = nullptr;
}

VkDescriptorPool& Vk_TextureManager::getDescriptorPool() {
    return m_descriptorPool;
}

VkDescriptorSetLayout& Vk_TextureManager::getDescriptorSetLayout() {
    return m_descriptorSetLayout;
}

Vk_Texture2D* Vk_TextureManager::getActiveTexture2D() {
    return reinterpret_cast<Vk_Texture2D*>(m_activeTexture);
}

std::unique_ptr<Texture2D> Vk_TextureManager::createTexture2D() {
    return std::make_unique<Vk_Texture2D>();
}

void Vk_TextureManager::useTexture2D(Texture2D* texture) {
    auto* castedTexture = reinterpret_cast<Vk_Texture2D*>(texture);
    castedTexture->use();
}

bool Vk_TextureManager::createDescriptorPool() {
    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.getVulkanDevice();

    VkResult result = VK_SUCCESS;

    VkDescriptorPoolSize poolSize = {
        .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = sMaxDescriptorSets,
    };

    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .maxSets = sMaxDescriptorSets,
        .poolSizeCount = 1,
        .pPoolSizes = &poolSize,
    };

    result = vkCreateDescriptorPool(device, &descriptorPoolCreateInfo, nullptr, &m_descriptorPool);
    if (result != VK_SUCCESS) {
        LogError(sTag, "Could not create descriptor pool");
        return false;
    }

    return true;
}

bool Vk_TextureManager::createDescriptorSetLayout() {
    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.getVulkanDevice();

    VkResult result = VK_SUCCESS;

    Vector<VkDescriptorSetLayoutBinding> layoutBindings = {
        // Diffuse texture
        {
            .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = nullptr,
        },
    };

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .bindingCount = static_cast<uint32>(layoutBindings.size()),
        .pBindings = layoutBindings.data(),
    };

    result = vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCreateInfo, nullptr, &m_descriptorSetLayout);
    if (result != VK_SUCCESS) {
        LogError(sTag, "Could not create descriptor set layout");
        return false;
    }

    return true;
}

}  // namespace engine::plugin::vulkan
