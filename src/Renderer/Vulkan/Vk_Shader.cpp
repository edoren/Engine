#include <System/LogManager.hpp>
#include <System/StringFormat.hpp>
#include <System/IO.hpp>

#include "Vk_Shader.hpp"

namespace engine {

namespace {

const String sTag("Vk_Shader");

VkShaderStageFlagBits sVkShaderTypes[] = {VK_SHADER_STAGE_VERTEX_BIT,
                                          VK_SHADER_STAGE_FRAGMENT_BIT};

}  // namespace

Vk_Shader::Vk_Shader() : m_module(VK_NULL_HANDLE) {}

Vk_Shader::Vk_Shader(Vk_Shader&& other) : m_module(other.m_module) {
    other.m_module = VK_NULL_HANDLE;
}

Vk_Shader::~Vk_Shader() {
    if (m_module) {
        VkDevice& device = Vk_Context::GetInstance().GetVulkanDevice();
        vkDestroyShaderModule(device, m_module, nullptr);
    }
}

Vk_Shader& Vk_Shader::operator=(Vk_Shader&& other) {
    m_module = other.m_module;
    other.m_module = VK_NULL_HANDLE;
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

    result = vkCreateShaderModule(device, &shader_module_create_info, nullptr,
                                  &m_module);
    if (result != VK_SUCCESS) {
        LogError(sTag, "Could not create shader module.");
        return false;
    }

    return true;
}

VkShaderModule& Vk_Shader::GetModule() {
    return m_module;
}

VkShaderStageFlagBits Vk_Shader::GetShaderType() {
    return sVkShaderTypes[0];
}

bool Vk_Shader::Link() {
    return true;
}

void Vk_Shader::Use() {}

}  // namespace engine
