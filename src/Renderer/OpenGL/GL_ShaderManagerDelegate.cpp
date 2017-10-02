#include "GL_ShaderManagerDelegate.hpp"
#include "GL_Shader.hpp"

namespace engine {

namespace {

const String sTag("GL_ShaderManagerDelegate");

const String sShaderFolder("glsl");

}  // namespace

GL_ShaderManagerDelegate::GL_ShaderManagerDelegate() {}

GL_ShaderManagerDelegate::~GL_ShaderManagerDelegate() {}

Shader* GL_ShaderManagerDelegate::CreateShader() {
    return new GL_Shader();
}

void GL_ShaderManagerDelegate::DeleteShader(Shader* shader) {
    delete shader;
}

void GL_ShaderManagerDelegate::SetActiveShader(Shader* shader) {
    GL_Shader* casted_shader = reinterpret_cast<GL_Shader*>(shader);
    casted_shader->Use();
}

const String& GL_ShaderManagerDelegate::GetShaderFolder() const {
    return sShaderFolder;
}

}  // namespace engine
