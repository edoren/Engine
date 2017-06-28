#pragma once

#include <Math/Math.hpp>
#include <Renderer/Shader.hpp>
#include <System/String.hpp>

#include "Vk_Config.hpp"
#include "Vk_Dependencies.hpp"
#include "Vk_Context.hpp"

namespace engine {

// class VULKAN_PLUGIN_API Vk_Shader : public Shader {
class VULKAN_PLUGIN_API Vk_Shader {
public:
    Vk_Shader();
    Vk_Shader(Vk_Shader&& other);
    ~Vk_Shader();

    Vk_Shader& operator=(Vk_Shader&& other);

    bool LoadFromMemory(const byte* source, std::size_t source_size,
                        ShaderType type);

    VkShaderModule& GetModule();

    VkShaderStageFlagBits GetShaderType();

    bool Link();

    void Use();

private:
    VkShaderModule m_module;
};

}  // namespace engine
