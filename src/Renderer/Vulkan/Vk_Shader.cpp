#include <System/LogManager.hpp>
#include <System/StringFormat.hpp>

#include "Vk_Context.hpp"
#include "Vk_Shader.hpp"

namespace engine {

namespace {

const String sTag("Vk_Shader");

}  // namespace

Vk_Shader::Vk_Shader() {
    for (size_t i = 0; i < m_modules.size(); i++) {
        m_modules[i] = VK_NULL_HANDLE;
    }
}

Vk_Shader::Vk_Shader(Vk_Shader&& other)
      : m_modules(std::move(other.m_modules)) {
    for (size_t i = 0; i < other.m_modules.size(); i++) {
        other.m_modules[i] = VK_NULL_HANDLE;
    }
}

Vk_Shader::~Vk_Shader() {
    VkDevice& device = Vk_Context::GetInstance().GetVulkanDevice();
    for (size_t i = 0; i < m_modules.size(); i++) {
        if (m_modules[i] != VK_NULL_HANDLE) {
            vkDestroyShaderModule(device, m_modules[i], nullptr);
        }
    }

    if (m_ubo_descriptor_set_layout != VK_NULL_HANDLE) {
        vkDestroyDescriptorSetLayout(device, m_ubo_descriptor_set_layout,
                                     nullptr);
        m_ubo_descriptor_set_layout = VK_NULL_HANDLE;
    }

    m_uniform_buffer.Destroy();
}

Vk_Shader& Vk_Shader::operator=(Vk_Shader&& other) {
    new (this) Vk_Shader(std::move(other));
    return *this;
}

bool Vk_Shader::LoadFromMemory(const byte* source, std::size_t source_size,
                               ShaderType type) {
    if (source_size == 0) {
        return false;
    }

    VkResult result = VK_SUCCESS;

    VkShaderModuleCreateInfo shader_module_create_info = {
        VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,  // sType
        nullptr,                                      // pNext
        VkShaderModuleCreateFlags(),                  // flags
        source_size,                                  // codeSize
        reinterpret_cast<const uint32_t*>(source)     // pCode
    };

    VkDevice& device = Vk_Context::GetInstance().GetVulkanDevice();

    size_t pos = static_cast<size_t>(type);

    if (m_modules[pos] != VK_NULL_HANDLE) {
        vkDestroyShaderModule(device, m_modules[pos], nullptr);
    }

    result = vkCreateShaderModule(device, &shader_module_create_info, nullptr,
                                  &m_modules[pos]);
    if (result != VK_SUCCESS) {
        LogError(sTag, "Could not create shader module");
        return false;
    }

    return true;
}

VkShaderModule& Vk_Shader::GetModule(ShaderType type) {
    return m_modules[static_cast<size_t>(type)];
}

VkDescriptorSet& Vk_Shader::GetUBODescriptorSet() {
    return m_ubo_descriptor_set;
}

VkDescriptorSetLayout& Vk_Shader::GetUBODescriptorSetLayout() {
    return m_ubo_descriptor_set_layout;
}

void Vk_Shader::SetDescriptor(json&& descriptor) {
    Shader::SetDescriptor(std::move(descriptor));

    if (!CreateUniformBuffer()) {
        LogError(sTag, "Could not create the UBO buffer");
        return;
    }

    if (!CreateUBODescriptorSetLayout()) {
        LogError(sTag, "Could not create the UBO descriptor set layout");
        return;
    }

    if (!AllocateUBODescriptorSet()) {
        LogError(sTag, "Could not create the UBO descriptor set");
        return;
    }

    if (!UpdateUBODescriptorSet()) {
        LogError(sTag, "Could not update the UBO descriptor set");
        return;
    }
}

bool Vk_Shader::CreateUBODescriptorSetLayout() {
    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.GetVulkanDevice();

    VkResult result = VK_SUCCESS;

    const json& bindings = GetDescriptor()["renderer"]["vulkan"]
                                          ["descriptor_set_layouts"]["bindings"];

    uint32 ubo_binding_position = 0;
    for (const auto& binding : bindings) {
        const json& pos = binding["pos"];
        const json& type = binding["type"];
        if (type.is_string() && type == "uniform_buffer") {
            ubo_binding_position = pos;
        }
    }

    VkDescriptorSetLayoutBinding layout_binding = {
        ubo_binding_position,               // binding
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,  // descriptorType
        1,                                  // descriptorCount
        VK_SHADER_STAGE_VERTEX_BIT,         // stageFlags
        nullptr                             // pImmutableSamplers
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
                                    nullptr, &m_ubo_descriptor_set_layout);
    if (result != VK_SUCCESS) {
        LogError(sTag, "Could not create descriptor set layout");
        return false;
    }

    return true;
}

bool Vk_Shader::AllocateUBODescriptorSet() {
    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.GetVulkanDevice();

    VkResult result = VK_SUCCESS;

    VkDescriptorSetAllocateInfo descriptor_set_allocate_info = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,  // sType
        nullptr,                                         // pNext
        context.GetUBODescriptorPool(),                  // descriptorPool
        1,                                               // descriptorSetCount
        &m_ubo_descriptor_set_layout                     // pSetLayouts
    };

    result = vkAllocateDescriptorSets(device, &descriptor_set_allocate_info,
                                      &m_ubo_descriptor_set);
    if (result != VK_SUCCESS) {
        LogError(sTag, "Could not allocate descriptor set");
        return false;
    }

    return true;
}

bool Vk_Shader::UpdateUBODescriptorSet() {
    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.GetVulkanDevice();

    VkDescriptorBufferInfo bufferInfo = {
        m_uniform_buffer.GetHandle(),  // buffer
        0,                             // offset
        m_ubo.GetDataSize()            // range
    };

    VkWriteDescriptorSet descriptor_writes = {
        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,  // sType
        nullptr,                                 // pNext
        m_ubo_descriptor_set,                    // dstSet
        0,                                       // dstBinding
        0,                                       // dstArrayElement
        1,                                       // descriptorCount
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,       // descriptorType
        nullptr,                                 // pImageInfo
        &bufferInfo,                             // pBufferInfo
        nullptr                                  // pTexelBufferView
    };

    vkUpdateDescriptorSets(device, 1, &descriptor_writes, 0, nullptr);

    return true;
}

bool Vk_Shader::CreateUniformBuffer() {
    return m_uniform_buffer.Create(m_ubo.GetDataSize(),
                                   VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                   (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
}

bool Vk_Shader::UpdateUniformBuffer() {
    if (m_uniform_buffer.GetHandle() == VK_NULL_HANDLE) return false;
    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.GetVulkanDevice();
    void* data;
    vkMapMemory(device, m_uniform_buffer.GetMemory(), 0, m_ubo.GetDataSize(), 0,
                &data);
    std::memcpy(data, m_ubo.GetData(), m_ubo.GetDataSize());
    vkUnmapMemory(device, m_uniform_buffer.GetMemory());
    return true;
}

}  // namespace engine
