#pragma once

#include <Renderer/ShaderManager.hpp>

#include "GL_Config.hpp"
#include "GL_Dependencies.hpp"

namespace engine {

class String;

class GL_Shader;

class OPENGL_PLUGIN_API GL_ShaderManager : public ShaderManager {
public:
    GL_ShaderManager();

    ~GL_ShaderManager() override;

    GL_Shader* loadFromFile(const String& basename);

    GL_Shader* loadFromMemory(const String& name, const std::map<ShaderType, String*>& shaderDataMap);

    GL_Shader* getShader(const String& name);

    GL_Shader* getActiveShader();

    static GL_ShaderManager& GetInstance();

    static GL_ShaderManager* GetInstancePtr();

protected:
    std::unique_ptr<Shader> createShader() override;

    void useShader(Shader* shader) override;

    const String& getShaderFolder() const override;
};

}  // namespace engine
