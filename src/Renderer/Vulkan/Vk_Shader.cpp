#include <System/LogManager.hpp>
#include <System/StringFormat.hpp>

#include "Vk_Context.hpp"
#include "Vk_Shader.hpp"

namespace engine {

namespace {

const String sTag("Vk_Shader");

const std::array<VkShaderStageFlagBits, 3> sVkShaderTypes = {
    VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT,
    VK_SHADER_STAGE_GEOMETRY_BIT};

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
}

Vk_Shader& Vk_Shader::operator=(Vk_Shader&& other) {
    m_modules = std::move(other.m_modules);
    for (size_t i = 0; i < other.m_modules.size(); i++) {
        other.m_modules[i] = VK_NULL_HANDLE;
    }
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
        LogError(sTag, "Could not create shader module.");
        return false;
    }

    return true;
}

VkShaderModule& Vk_Shader::GetModule(ShaderType type) {
    return m_modules[static_cast<size_t>(type)];
}

}  // namespace engine
