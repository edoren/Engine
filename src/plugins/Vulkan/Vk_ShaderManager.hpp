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

    ~Vk_ShaderManager() override;

    Vk_Shader* loadFromFile(const String& basename);

    Vk_Shader* loadFromMemory(const String& name, std::map<ShaderType, String*> shader_data_map);

    Vk_Shader* getShader(const String& name);

    Vk_Shader* getActiveShader();

    static Vk_ShaderManager& GetInstance();

    static Vk_ShaderManager* GetInstancePtr();

protected:
    std::unique_ptr<Shader> createShader() override;

    void useShader(Shader* shader) override;

    const String& getShaderFolder() const override;
};

}  // namespace engine
