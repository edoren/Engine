#include <System/LogManager.hpp>
#include <System/StringFormat.hpp>
#include <System/IO.hpp>

#include "Vk_Shader.hpp"

namespace engine {

namespace {

vk::ShaderStageFlagBits s_vk_shader_types[] = {
    vk::ShaderStageFlagBits::eVertex, vk::ShaderStageFlagBits::eFragment};

}  // namespace

Vk_Shader::Vk_Shader() {}

Vk_Shader::Vk_Shader(Vk_Shader&& other) : m_module(other.m_module) {}

Vk_Shader::~Vk_Shader() {
    if (m_module) {
        vk::Device& device = Vk_Context::GetInstance().GetVulkanDevice();
        device.destroyShaderModule(m_module, nullptr);
    }
}

Vk_Shader& Vk_Shader::operator=(Vk_Shader&& other) {
    m_module = other.m_module;
    other.m_module = nullptr;
    return *this;
}

bool Vk_Shader::LoadFromMemory(const byte* source, std::size_t source_size,
                               ShaderType type) {
    if (source_size == 0) {
        return false;
    }

    vk::Result result;

    vk::ShaderModuleCreateInfo shader_module_create_info{
        vk::ShaderModuleCreateFlags(),             // flags
        source_size,                               // codeSize
        reinterpret_cast<const uint32_t*>(source)  // pCode
    };

    vk::Device& device = Vk_Context::GetInstance().GetVulkanDevice();
    result = device.createShaderModule(&shader_module_create_info, nullptr,
                                       &m_module);
    if (result != vk::Result::eSuccess) {
        LogError("Vk_RenderWindow", "Could not create shader module.");
        return false;
    }

    return true;
}

vk::ShaderModule& Vk_Shader::GetModule() {
    return m_module;
}

vk::ShaderStageFlagBits Vk_Shader::GetShaderType() {
    return vk::ShaderStageFlagBits::eVertex;
}

bool Vk_Shader::Link() {
    return true;
}

void Vk_Shader::Use() {}

}  // namespace engine
