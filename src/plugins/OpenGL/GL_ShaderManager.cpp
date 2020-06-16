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

GL_ShaderManager::GL_ShaderManager() {
    ShaderManager& base_instance = ShaderManager::GetInstance();
    sDerivedInstance = reinterpret_cast<GL_ShaderManager*>(&base_instance);
}

GL_ShaderManager::~GL_ShaderManager() {
    sDerivedInstance = nullptr;
}

GL_Shader* GL_ShaderManager::loadFromFile(const String& basename) {
    Shader* base_shader = ShaderManager::loadFromFile(basename);
    return reinterpret_cast<GL_Shader*>(base_shader);
}

GL_Shader* GL_ShaderManager::loadFromMemory(const String& name, const std::map<ShaderType, String*>& shader_data_map) {
    Shader* base_shader = ShaderManager::loadFromMemory(name, shader_data_map);
    return reinterpret_cast<GL_Shader*>(base_shader);
}

GL_Shader* GL_ShaderManager::getShader(const String& name) {
    Shader* base_shader = ShaderManager::getShader(name);
    return reinterpret_cast<GL_Shader*>(base_shader);
}

GL_Shader* GL_ShaderManager::getActiveShader() {
    return reinterpret_cast<GL_Shader*>(m_activeShader);
}

std::unique_ptr<Shader> GL_ShaderManager::createShader() {
    return std::make_unique<GL_Shader>();
}

void GL_ShaderManager::useShader(Shader* shader) {
    auto* casted_shader = reinterpret_cast<GL_Shader*>(shader);
    casted_shader->use();
}

const String& GL_ShaderManager::getShaderFolder() const {
    return sShaderFolder;
}

}  // namespace engine
