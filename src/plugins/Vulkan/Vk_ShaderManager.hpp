#pragma once

#include <Renderer/ShaderManager.hpp>

#include "Vk_Config.hpp"
#include "Vk_Dependencies.hpp"

namespace engine {

class String;

class Vk_Shader;

class VULKAN_PLUGIN_API Vk_ShaderManager : public ShaderManager {
public:
    Vk_ShaderManager();

    ~Vk_ShaderManager();

    Vk_Shader* LoadFromFile(const String& basename);

    Vk_Shader* LoadFromMemory(const String& name, std::map<ShaderType, String*> shader_data_map);

    Vk_Shader* GetShader(const String& name);

    Vk_Shader* GetActiveShader();

    static Vk_ShaderManager& GetInstance();

    static Vk_ShaderManager* GetInstancePtr();

protected:
    Shader* CreateShader() override;

    void DeleteShader(Shader* shader) override;

    void UseShader(Shader* shader) override;

    const String& GetShaderFolder() const override;
};

}  // namespace engine
