#pragma once

#include <Math/Math.hpp>
#include <Renderer/Shader.hpp>
#include <System/String.hpp>

#include "Vk_Config.hpp"
#include "Vk_Buffer.hpp"
#include "Vk_Dependencies.hpp"

namespace engine {

class VULKAN_PLUGIN_API Vk_Shader : public Shader {
public:
    Vk_Shader();
    Vk_Shader(Vk_Shader&& other);
    ~Vk_Shader() override;

    Vk_Shader& operator=(Vk_Shader&& other);

    bool LoadFromMemory(const byte* source, std::size_t source_size,
                        ShaderType type) override;

    VkShaderModule& GetModule(ShaderType type);

    VkShaderStageFlagBits GetShaderType();

private:
    std::array<VkShaderModule, sShaderTypeCount> m_modules;

    Vk_Buffer m_uniform_buffer;
};

}  // namespace engine
