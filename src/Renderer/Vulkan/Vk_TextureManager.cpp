#include <System/LogManager.hpp>

#include "Vk_Context.hpp"
#include "Vk_Texture2D.hpp"
#include "Vk_TextureManager.hpp"

namespace engine {

namespace {

const String sTag("Vk_TextureManager");

const uint32 sDescriptorSetBinding(0);

const uint32 sMaxDescriptorSets(16);

Vk_TextureManager* sDerivedInstance = nullptr;

}  // namespace

Vk_TextureManager& Vk_TextureManager::GetInstance() {
    assert(sDerivedInstance);
    return (*sDerivedInstance);
}

Vk_TextureManager* Vk_TextureManager::GetInstancePtr() {
    return sDerivedInstance;
}

Vk_TextureManager::Vk_TextureManager()
      : TextureManager(),
        m_descriptor_pool(VK_NULL_HANDLE),
        m_descriptor_set_layout(VK_NULL_HANDLE) {
    TextureManager& base_instance = TextureManager::GetInstance();
    sDerivedInstance = reinterpret_cast<Vk_TextureManager*>(&base_instance);

    CreateDescriptorPool();
    CreateDescriptorSetLayout();
}

Vk_TextureManager::~Vk_TextureManager() {
    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.GetVulkanDevice();

    for (auto texture_pair : m_textures) {
        delete texture_pair.second;
    }
    m_textures.clear();

    if (m_descriptor_pool != VK_NULL_HANDLE) {
        vkDestroyDescriptorPool(device, m_descriptor_pool, nullptr);
        m_descriptor_pool = VK_NULL_HANDLE;
    }

    if (m_descriptor_set_layout != VK_NULL_HANDLE) {
        vkDestroyDescriptorSetLayout(device, m_descriptor_set_layout, nullptr);
        m_descriptor_set_layout = VK_NULL_HANDLE;
    }

    sDerivedInstance = nullptr;
}

VkDescriptorPool& Vk_TextureManager::GetDescriptorPool() {
    return m_descriptor_pool;
}

VkDescriptorSetLayout& Vk_TextureManager::GetDescriptorSetLayout() {
    return m_descriptor_set_layout;
}

Vk_Texture2D* Vk_TextureManager::GetActiveTexture2D() {
    return reinterpret_cast<Vk_Texture2D*>(m_active_texture);
}

void Vk_TextureManager::UseTexture2D(Texture2D* texture) {
    Vk_Texture2D* casted_texture = reinterpret_cast<Vk_Texture2D*>(texture);
    casted_texture->Use();
}

bool Vk_TextureManager::CreateDescriptorPool() {
    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.GetVulkanDevice();

    VkResult result = VK_SUCCESS;

    VkDescriptorPoolSize pool_size = {
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,  // type
        sMaxDescriptorSets                          // descriptorCount
    };

    VkDescriptorPoolCreateInfo descriptor_pool_create_info = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,  // sType
        nullptr,                                        // pNext
        0,                                              // flags
        sMaxDescriptorSets,                             // maxSets
        1,                                              // poolSizeCount
        &pool_size                                      // pPoolSizes
    };

    result = vkCreateDescriptorPool(device, &descriptor_pool_create_info,
                                    nullptr, &m_descriptor_pool);
    if (result != VK_SUCCESS) {
        LogError(sTag, "Could not create descriptor pool");
        return false;
    }

    return true;
}

bool Vk_TextureManager::CreateDescriptorSetLayout() {
    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.GetVulkanDevice();

    VkResult result = VK_SUCCESS;

    VkDescriptorSetLayoutBinding layout_binding = {
        sDescriptorSetBinding,                      // binding
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,  // descriptorType
        1,                                          // descriptorCount
        VK_SHADER_STAGE_FRAGMENT_BIT,               // stageFlags
        nullptr                                     // pImmutableSamplers
    };

    VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,  // sType
        nullptr,                                              // pNext
        0,                                                    // flags
        1,                                                    // bindingCount
        &layout_binding                                       // pBindings
    };

    result =
        vkCreateDescriptorSetLayout(device, &descriptor_set_layout_create_info,
                                    nullptr, &m_descriptor_set_layout);
    if (result != VK_SUCCESS) {
        LogError(sTag, "Could not create descriptor set layout");
        return false;
    }

    return true;
}

}  // namespace engine
