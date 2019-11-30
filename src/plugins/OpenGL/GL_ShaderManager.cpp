#include "GL_ShaderManager.hpp"
#include "GL_Shader.hpp"

namespace engine {

namespace {

const String sTag("GL_ShaderManager");

const String sShaderFolder("glsl");

GL_ShaderManager* sDerivedInstance = nullptr;

}  // namespace

GL_ShaderManager& GL_ShaderManager::GetInstance() {
    assert(sDerivedInstance);
    return (*sDerivedInstance);
}

GL_ShaderManager* GL_ShaderManager::GetInstancePtr() {
    return sDerivedInstance;
}

GL_ShaderManager::GL_ShaderManager() : ShaderManager() {
    ShaderManager& base_instance = ShaderManager::GetInstance();
    sDerivedInstance = reinterpret_cast<GL_ShaderManager*>(&base_instance);
}

GL_ShaderManager::~GL_ShaderManager() {
    for (auto shader_pair : m_shaders) {
        DeleteShader(shader_pair.second);
    }
    m_shaders.clear();
    sDerivedInstance = nullptr;
}

GL_Shader* GL_ShaderManager::LoadFromFile(const String& basename) {
    Shader* base_shader = ShaderManager::LoadFromFile(basename);
    return reinterpret_cast<GL_Shader*>(base_shader);
}

GL_Shader* GL_ShaderManager::LoadFromMemory(const String& name, std::map<ShaderType, String*> shader_data_map) {
    Shader* base_shader = ShaderManager::LoadFromMemory(name, shader_data_map);
    return reinterpret_cast<GL_Shader*>(base_shader);
}

GL_Shader* GL_ShaderManager::GetShader(const String& name) {
    Shader* base_shader = ShaderManager::GetShader(name);
    return reinterpret_cast<GL_Shader*>(base_shader);
}

GL_Shader* GL_ShaderManager::GetActiveShader() {
    return reinterpret_cast<GL_Shader*>(m_active_shader);
}

Shader* GL_ShaderManager::CreateShader() {
    return new GL_Shader();
}

void GL_ShaderManager::DeleteShader(Shader* shader) {
    delete shader;
}

void GL_ShaderManager::UseShader(Shader* shader) {
    GL_Shader* casted_shader = reinterpret_cast<GL_Shader*>(shader);
    casted_shader->Use();
}

const String& GL_ShaderManager::GetShaderFolder() const {
    return sShaderFolder;
}

}  // namespace engine
