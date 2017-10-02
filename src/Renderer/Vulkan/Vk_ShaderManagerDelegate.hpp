#pragma once

#include <Renderer/ShaderManager.hpp>

#include "Vk_Config.hpp"
#include "Vk_Dependencies.hpp"

namespace engine {

class String;

class VULKAN_PLUGIN_API Vk_ShaderManagerDelegate
    : public ShaderManagerDelegate {
public:
    Vk_ShaderManagerDelegate();

    ~Vk_ShaderManagerDelegate();

    Shader* CreateShader() override;

    void DeleteShader(Shader* shader) override;

    void SetActiveShader(Shader* shader) override;

    const String& GetShaderFolder() const override;
};

}  // namespace engine
