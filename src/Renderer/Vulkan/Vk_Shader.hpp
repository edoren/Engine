#pragma once

#include <Math/Math.hpp>
#include <Renderer/Shader.hpp>
#include <System/String.hpp>

#include "Vk_Config.hpp"
#include "Vk_Dependencies.hpp"

namespace engine {

class VULKAN_PLUGIN_API Vk_Shader : public Shader {
public:
    Vk_Shader();
    Vk_Shader(Vk_Shader&& other);
    ~Vk_Shader();

    Vk_Shader& operator=(Vk_Shader&& other);

    bool LoadFromMemory(const byte* source, std::size_t source_size,
                        ShaderType type) override;

    bool Link() override;

    void Use() override;

    VkShaderModule& GetModule();

    VkShaderStageFlagBits GetShaderType();

private:
    VkShaderModule m_module;
};

}  // namespace engine
