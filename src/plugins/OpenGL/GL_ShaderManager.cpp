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
    ShaderManager& baseInstance = ShaderManager::GetInstance();
    sDerivedInstance = reinterpret_cast<GL_ShaderManager*>(&baseInstance);
}

GL_ShaderManager::~GL_ShaderManager() {
    sDerivedInstance = nullptr;
}

GL_Shader* GL_ShaderManager::loadFromFile(const String& basename) {
    Shader* baseShader = ShaderManager::loadFromFile(basename);
    return reinterpret_cast<GL_Shader*>(baseShader);
}

GL_Shader* GL_ShaderManager::loadFromMemory(const String& name, const std::map<ShaderType, String*>& shaderDataMap) {
    Shader* baseShader = ShaderManager::loadFromMemory(name, shaderDataMap);
    return reinterpret_cast<GL_Shader*>(baseShader);
}

GL_Shader* GL_ShaderManager::getShader(const String& name) {
    Shader* baseShader = ShaderManager::getShader(name);
    return reinterpret_cast<GL_Shader*>(baseShader);
}

GL_Shader* GL_ShaderManager::getActiveShader() {
    return reinterpret_cast<GL_Shader*>(m_activeShader);
}

std::unique_ptr<Shader> GL_ShaderManager::createShader() {
    return std::make_unique<GL_Shader>();
}

void GL_ShaderManager::useShader(Shader* shader) {
    auto* castedShader = reinterpret_cast<GL_Shader*>(shader);
    castedShader->use();
}

const String& GL_ShaderManager::getShaderFolder() const {
    return sShaderFolder;
}

}  // namespace engine
