#pragma once

#include <Renderer/ShaderManager.hpp>

#include "GL_Config.hpp"
#include "GL_Dependencies.hpp"

namespace engine {

class String;

class OPENGL_PLUGIN_API GL_ShaderManagerDelegate
    : public ShaderManagerDelegate {
public:
    GL_ShaderManagerDelegate();

    ~GL_ShaderManagerDelegate();

    Shader* CreateShader() override;

    void DeleteShader(Shader* shader) override;

    void SetActiveShader(Shader* shader) override;

    const String& GetShaderFolder() const override;
};

}  // namespace engine
