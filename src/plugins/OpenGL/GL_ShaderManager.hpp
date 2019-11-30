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

    ~GL_ShaderManager();

    GL_Shader* LoadFromFile(const String& basename);

    GL_Shader* LoadFromMemory(const String& name, std::map<ShaderType, String*> shader_data_map);

    GL_Shader* GetShader(const String& name);

    GL_Shader* GetActiveShader();

    static GL_ShaderManager& GetInstance();

    static GL_ShaderManager* GetInstancePtr();

protected:
    Shader* CreateShader() override;

    void DeleteShader(Shader* shader) override;

    void UseShader(Shader* shader) override;

    const String& GetShaderFolder() const override;
};

}  // namespace engine
