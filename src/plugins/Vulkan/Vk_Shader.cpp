#include <System/LogManager.hpp>
#include <System/StringFormat.hpp>

#include "Vk_Context.hpp"
#include "Vk_Shader.hpp"

namespace engine {

namespace {

const String sTag("Vk_Shader");

}  // namespace

Vk_Shader::Vk_Shader() {
    for (auto& module : m_modules) {
        module = VK_NULL_HANDLE;
    }
}

Vk_Shader::Vk_Shader(Vk_Shader&& other) noexcept : m_modules(other.m_modules) {
    for (auto& module : other.m_modules) {
        module = VK_NULL_HANDLE;
    }
}

Vk_Shader::~Vk_Shader() {
    VkDevice& device = Vk_Context::GetInstance().getVulkanDevice();
    for (auto& module : m_modules) {
        if (module != VK_NULL_HANDLE) {
            vkDestroyShaderModule(device, module, nullptr);
        }
    }

    if (m_uboDescriptorSetLayout != VK_NULL_HANDLE) {
        vkDestroyDescriptorSetLayout(device, m_uboDescriptorSetLayout, nullptr);
        m_uboDescriptorSetLayout = VK_NULL_HANDLE;
    }

    m_uniformBuffers._static.destroy();
}

Vk_Shader& Vk_Shader::operator=(Vk_Shader&& other) noexcept {
    new (this) Vk_Shader(std::move(other));
    return *this;
}

bool Vk_Shader::loadFromMemory(const byte* source, std::size_t source_size, ShaderType type) {
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

    VkDevice& device = Vk_Context::GetInstance().getVulkanDevice();

    auto pos = static_cast<size_t>(type);

    if (m_modules[pos] != VK_NULL_HANDLE) {
        vkDestroyShaderModule(device, m_modules[pos], nullptr);
    }

    result = vkCreateShaderModule(device, &shader_module_create_info, nullptr, &m_modules[pos]);
    if (result != VK_SUCCESS) {
        LogError(sTag, "Could not create shader module");
        return false;
    }

    return true;
}

VkShaderModule& Vk_Shader::getModule(ShaderType type) {
    return m_modules[static_cast<size_t>(type)];
}

VkDescriptorSet& Vk_Shader::getUboDescriptorSet() {
    return m_uboDescriptorSet;
}

VkDescriptorSetLayout& Vk_Shader::getUboDescriptorSetLayout() {
    return m_uboDescriptorSetLayout;
}

void Vk_Shader::setDescriptor(json&& descriptor) {
    m_descriptor = std::move(descriptor);

    String descriptor_name = m_descriptor["name"];

    std::vector<UniformBufferObject::Item> attributes;
    for (auto& attribute : m_descriptor["uniform_buffer"]["attributes"]) {
        String name = attribute["name"];
        String type = attribute["type"];
        attributes.push_back({name, getUboDataTypeFromString(type)});
    }
    m_ubo.setAttributes(attributes);

    attributes.clear();
    for (auto& attribute : m_descriptor["uniform_buffer_dynamic"]["attributes"]) {
        String name = attribute["name"];
        String type = attribute["type"];
        attributes.push_back({name, getUboDataTypeFromString(type)});
    }
    m_uboDynamic.setAttributes(attributes);

    std::vector<VertexLayout::Component> vertex_inputs;
    for (auto& component : m_descriptor["vertex_layout"]["vertex_input"]) {
        if (component == "position") {
            vertex_inputs.push_back(VertexLayout::Component::POSITION);
        } else if (component == "color") {
            vertex_inputs.push_back(VertexLayout::Component::COLOR);
        } else if (component == "normal") {
            vertex_inputs.push_back(VertexLayout::Component::NORMAL);
        } else if (component == "uv") {
            vertex_inputs.push_back(VertexLayout::Component::UV);
        } else {
            LogFatal(sTag,
                     "Error invalid VertexLayout Component '{}', please check the vertex_layout "
                     "in the '{}.json' shader descriptor"_format(component, descriptor_name));
            return;
        }
    }
    m_vertexLayout = std::move(vertex_inputs);

    if (!createUniformBuffers()) {
        LogError(sTag, "Could not create the UBO buffer");
        return;
    }

    if (!createUboDescriptorSetLayout()) {
        LogError(sTag, "Could not create the UBO descriptor set layout");
        return;
    }

    if (!allocateUboDescriptorSet()) {
        LogError(sTag, "Could not create the UBO descriptor set");
        return;
    }

    if (!updateUboDescriptorSet()) {
        LogError(sTag, "Could not update the UBO descriptor set");
        return;
    }
}

bool Vk_Shader::createUboDescriptorSetLayout() {
    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.getVulkanDevice();

    VkResult result = VK_SUCCESS;

    const json& bindings = m_descriptor["renderer"]["vulkan"]["descriptor_set_layouts"]["bindings"];

    std::vector<VkDescriptorSetLayoutBinding> layout_bindings;
    for (const auto& binding : bindings) {
        const json& pos = binding["pos"];
        const json& type = binding["type"];
        if (pos.is_number_integer() && type.is_string()) {
            uint32 ubo_binding_position = pos;
            VkDescriptorType descriptor_type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

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
        vkCreateDescriptorSetLayout(device, &descriptor_set_layout_create_info, nullptr, &m_uboDescriptorSetLayout);
    if (result != VK_SUCCESS) {
        LogError(sTag, "Could not create descriptor set layout");
        return false;
    }

    return true;
}

bool Vk_Shader::allocateUboDescriptorSet() {
    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.getVulkanDevice();

    VkResult result = VK_SUCCESS;

    VkDescriptorSetAllocateInfo descriptor_set_allocate_info = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,  // sType
        nullptr,                                         // pNext
        context.getUboDescriptorPool(),                  // descriptorPool
        1,                                               // descriptorSetCount
        &m_uboDescriptorSetLayout                        // pSetLayouts
    };

    result = vkAllocateDescriptorSets(device, &descriptor_set_allocate_info, &m_uboDescriptorSet);
    if (result != VK_SUCCESS) {
        LogError(sTag, "Could not allocate descriptor set");
        return false;
    }

    return true;
}

bool Vk_Shader::updateUboDescriptorSet() {
    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.getVulkanDevice();

    LogDebug(sTag, "Buffer UBO Dynamic Size: {}"_format(m_uniformBuffers.dynamic.getSize()));

    std::array<VkDescriptorBufferInfo, 2> bufferInfos = {{
        {
            m_uniformBuffers._static.getHandle(),  // buffer
            0,                                     // offset
            VK_WHOLE_SIZE                          // range
        },
        {
            m_uniformBuffers.dynamic.getHandle(),  // buffer
            0,                                     // offset
            VK_WHOLE_SIZE                          // range
        },
    }};

    std::array<VkWriteDescriptorSet, 2> descriptor_writes = {{
        {
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,  // sType
            nullptr,                                 // pNext
            m_uboDescriptorSet,                      // dstSet
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
            m_uboDescriptorSet,                         // dstSet
            1,                                          // dstBinding
            0,                                          // dstArrayElement
            1,                                          // descriptorCount
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,  // descriptorType
            nullptr,                                    // pImageInfo
            &bufferInfos[1],                            // pBufferInfo
            nullptr                                     // pTexelBufferView
        },
    }};

    vkUpdateDescriptorSets(device, static_cast<uint32>(descriptor_writes.size()), descriptor_writes.data(), 0, nullptr);

    return true;
}

UniformBufferObject& Vk_Shader::getUbo() {
    return m_ubo;
}

UniformBufferObject& Vk_Shader::getUboDynamic() {
    return m_uboDynamic;
}

const Vk_VertexLayout& Vk_Shader::getVertexLayout() const {
    return m_vertexLayout;
}

bool Vk_Shader::createUniformBuffers() {
    bool result = true;

    // Static UBO memory buffer
    result &=
        m_uniformBuffers._static.create(m_ubo.getDataSize(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                        (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));

    // Dynamic UBO memory buffer
    PhysicalDeviceParameters& physical_device = Vk_Context::GetInstance().getPhysicalDevice();
    auto minUboAlignment = static_cast<size_t>(physical_device.properties.limits.minUniformBufferOffsetAlignment);

    m_uboDynamic.setBufferSize(50, minUboAlignment);  // TODO: CHANGE THIS

    result &= m_uniformBuffers.dynamic.create(m_uboDynamic.getDataSize(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    return result;
}

bool Vk_Shader::uploadUniformBuffers() {
    if (m_uniformBuffers._static.getHandle() == VK_NULL_HANDLE ||
        m_uniformBuffers.dynamic.getHandle() == VK_NULL_HANDLE) {
        return false;
    }
    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.getVulkanDevice();

    {
        void* data;
        vkMapMemory(device, m_uniformBuffers._static.getMemory(), 0, m_ubo.getDataSize(), 0, &data);
        std::memcpy(data, m_ubo.getData(), m_ubo.getDataSize());
        vkUnmapMemory(device, m_uniformBuffers._static.getMemory());
    }

    {
        void* data;
        vkMapMemory(device, m_uniformBuffers.dynamic.getMemory(), 0, m_uboDynamic.getDataSize(), 0, &data);
        std::memcpy(data, m_uboDynamic.getData(), m_uboDynamic.getDataSize());

        VkMappedMemoryRange memoryRange = {
            VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,  // sType;
            nullptr,                                // pNext;
            m_uniformBuffers.dynamic.getMemory(),   // memory;
            0,                                      // offset;
            m_uboDynamic.getDataSize()              // size;
        };
        vkFlushMappedMemoryRanges(device, 1, &memoryRange);

        vkUnmapMemory(device, m_uniformBuffers.dynamic.getMemory());
    }

    return true;
}

}  // namespace engine
