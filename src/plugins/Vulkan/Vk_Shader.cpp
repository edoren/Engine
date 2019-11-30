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

    m_uniform_buffers._static.Destroy();
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
    m_descriptor = std::move(descriptor);

    std::vector<UniformBufferObject::Item> attributes;
    for (auto& attribute : m_descriptor["uniform_buffer"]["attributes"]) {
        String name = attribute["name"];
        String type = attribute["type"];
        attributes.push_back({name, GetUBODataTypeFromString(type)});
    }
    m_ubo.SetAttributes(attributes);

    attributes.clear();
    for (auto& attribute :
         m_descriptor["uniform_buffer_dynamic"]["attributes"]) {
        String name = attribute["name"];
        String type = attribute["type"];
        attributes.push_back({name, GetUBODataTypeFromString(type)});
    }
    m_ubo_dynamic.SetAttributes(attributes);

    if (!CreateUniformBuffers()) {
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

    const json& bindings = m_descriptor["renderer"]["vulkan"]
                                       ["descriptor_set_layouts"]["bindings"];

    std::vector<VkDescriptorSetLayoutBinding> layout_bindings;
    for (const auto& binding : bindings) {
        const json& pos = binding["pos"];
        const json& type = binding["type"];
        if (pos.is_number_integer() && type.is_string()) {
            uint32 ubo_binding_position = pos;
            VkDescriptorType descriptor_type =
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

            if (type == "uniform_buffer") {
                descriptor_type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            } else if (type == "uniform_buffer_dynamic") {
                descriptor_type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
            }

            layout_bindings.push_back({
                ubo_binding_position,        // binding
                descriptor_type,             // descriptorType
                1,                           // descriptorCount
                VK_SHADER_STAGE_VERTEX_BIT,  // stageFlags
                nullptr                      // pImmutableSamplers
            });
        }
    }

    VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,  // sType
        nullptr,                                              // pNext
        0,                                                    // flags
        static_cast<uint32>(layout_bindings.size()),          // bindingCount
        layout_bindings.data()                                // pBindings
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

    LogDebug(sTag, "Buffer UBO Dynamic Size: {}"_format(
                       m_uniform_buffers._dynamic.GetSize()));

    std::array<VkDescriptorBufferInfo, 2> bufferInfos = {{
        {
            m_uniform_buffers._static.GetHandle(),  // buffer
            0,                                      // offset
            VK_WHOLE_SIZE                           // range
        },
        {
            m_uniform_buffers._dynamic.GetHandle(),  // buffer
            0,                                       // offset
            VK_WHOLE_SIZE                            // range
        },
    }};

    std::array<VkWriteDescriptorSet, 2> descriptor_writes = {{
        {
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,  // sType
            nullptr,                                 // pNext
            m_ubo_descriptor_set,                    // dstSet
            0,                                       // dstBinding
            0,                                       // dstArrayElement
            1,                                       // descriptorCount
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,       // descriptorType
            nullptr,                                 // pImageInfo
            &bufferInfos[0],                         // pBufferInfo
            nullptr                                  // pTexelBufferView
        },
        {
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,     // sType
            nullptr,                                    // pNext
            m_ubo_descriptor_set,                       // dstSet
            1,                                          // dstBinding
            0,                                          // dstArrayElement
            1,                                          // descriptorCount
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,  // descriptorType
            nullptr,                                    // pImageInfo
            &bufferInfos[1],                            // pBufferInfo
            nullptr                                     // pTexelBufferView
        },
    }};

    vkUpdateDescriptorSets(device,
                           static_cast<uint32>(descriptor_writes.size()),
                           descriptor_writes.data(), 0, nullptr);

    return true;
}

UniformBufferObject& Vk_Shader::GetUBO() {
    return m_ubo;
}

UniformBufferObject& Vk_Shader::GetUBODynamic() {
    return m_ubo_dynamic;
}

bool Vk_Shader::CreateUniformBuffers() {
    bool result = true;

    // Static UBO memory buffer
    result &= m_uniform_buffers._static.Create(
        m_ubo.GetDataSize(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));

    // Dynamic UBO memory buffer
    PhysicalDeviceParameters& physical_device =
        Vk_Context::GetInstance().GetPhysicalDevice();
    size_t minUboAlignment = static_cast<size_t>(
        physical_device.properties.limits.minUniformBufferOffsetAlignment);

    m_ubo_dynamic.SetBufferSize(50, minUboAlignment);  // TODO: CHANGE THIS

    result &= m_uniform_buffers._dynamic.Create(
        m_ubo_dynamic.GetDataSize(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    return result;
}

bool Vk_Shader::UploadUniformBuffers() {
    if (m_uniform_buffers._static.GetHandle() == VK_NULL_HANDLE ||
        m_uniform_buffers._dynamic.GetHandle() == VK_NULL_HANDLE)
        return false;
    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.GetVulkanDevice();

    {
        void* data;
        vkMapMemory(device, m_uniform_buffers._static.GetMemory(), 0,
                    m_ubo.GetDataSize(), 0, &data);
        std::memcpy(data, m_ubo.GetData(), m_ubo.GetDataSize());
        vkUnmapMemory(device, m_uniform_buffers._static.GetMemory());
    }

    {
        void* data;
        vkMapMemory(device, m_uniform_buffers._dynamic.GetMemory(), 0,
                    m_ubo_dynamic.GetDataSize(), 0, &data);
        std::memcpy(data, m_ubo_dynamic.GetData(), m_ubo_dynamic.GetDataSize());

        VkMappedMemoryRange memoryRange = {
            VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,   // sType;
            nullptr,                                 // pNext;
            m_uniform_buffers._dynamic.GetMemory(),  // memory;
            0,                                       // offset;
            m_ubo_dynamic.GetDataSize()              // size;
        };
        vkFlushMappedMemoryRanges(device, 1, &memoryRange);

        vkUnmapMemory(device, m_uniform_buffers._dynamic.GetMemory());
    }

    return true;
}

}  // namespace engine
