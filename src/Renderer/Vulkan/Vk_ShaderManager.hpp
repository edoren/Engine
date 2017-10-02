#pragma once

#include <Renderer/ShaderManager.hpp>

#include "Vk_Config.hpp"
#include "Vk_Dependencies.hpp"

namespace engine {

class String;

class VULKAN_PLUGIN_API Vk_ShaderManager : public ShaderManager {
public:
    Vk_ShaderManager();

    ~Vk_ShaderManager();

protected:
    Shader* CreateShader() override;

    void DeleteShader(Shader* shader) override;

    void SetActiveShader(Shader* shader) override;

    const String& GetShaderFolder() const override;
};

}  // namespace engine
