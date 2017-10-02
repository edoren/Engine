#include "GL_Shader.hpp"
#include "GL_ShaderManager.hpp"

namespace engine {

namespace {

const String sTag("GL_ShaderManager");

const String sShaderFolder("glsl");

}  // namespace

GL_ShaderManager::GL_ShaderManager() {}

GL_ShaderManager::~GL_ShaderManager() {
    for (auto shader_pair : m_shaders) {
        DeleteShader(shader_pair.second);
    }
    m_shaders.clear();
}

Shader* GL_ShaderManager::CreateShader() {
    return new GL_Shader();
}

void GL_ShaderManager::DeleteShader(Shader* shader) {
    delete shader;
}

void GL_ShaderManager::SetActiveShader(Shader* shader) {
    GL_Shader* casted_shader = reinterpret_cast<GL_Shader*>(shader);
    casted_shader->Use();
}

const String& GL_ShaderManager::GetShaderFolder() const {
    return sShaderFolder;
}

}  // namespace engine
