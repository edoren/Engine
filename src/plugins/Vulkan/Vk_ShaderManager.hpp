#pragma once

#include <Renderer/ShaderManager.hpp>

#include "Vk_Config.hpp"
#include "Vk_Dependencies.hpp"

#include <map>
#include <memory>

namespace engine::plugin::vulkan {

class Vk_Shader;

class VULKAN_PLUGIN_API Vk_ShaderManager : public ShaderManager {
public:
    Vk_ShaderManager();

    ~Vk_ShaderManager() override;

    Vk_Shader* loadFromFile(const String& basename);

    Vk_Shader* loadFromMemory(const String& name, const std::map<ShaderType, String*>& shaderDataMap);

    Vk_Shader* getShader(const String& name);

    Vk_Shader* getActiveShader();

    /**
     * @copydoc ShaderManager::GetInstance
     */
    static Vk_ShaderManager& GetInstance();

    /**
     * @copydoc ShaderManager::GetInstance
     */
    static Vk_ShaderManager* GetInstancePtr();

protected:
    std::unique_ptr<Shader> createShader() override;

    void useShader(Shader* shader) override;

    const String& getShaderFolder() const override;
};

}  // namespace engine::plugin::vulkan
