#include "Vk_ShaderManager.hpp"

#include "Vk_Shader.hpp"

namespace engine {

namespace {

const String sTag("Vk_ShaderManager");

const String sShaderFolder("spirv");

Vk_ShaderManager* sDerivedInstance = nullptr;

}  // namespace

Vk_ShaderManager& Vk_ShaderManager::GetInstance() {
    assert(sDerivedInstance);
    return (*sDerivedInstance);
}

Vk_ShaderManager* Vk_ShaderManager::GetInstancePtr() {
    return sDerivedInstance;
}

Vk_ShaderManager::Vk_ShaderManager() {
    ShaderManager& baseInstance = ShaderManager::GetInstance();
    sDerivedInstance = reinterpret_cast<Vk_ShaderManager*>(&baseInstance);
}

Vk_ShaderManager::~Vk_ShaderManager() {
    sDerivedInstance = nullptr;
}

Vk_Shader* Vk_ShaderManager::loadFromFile(const String& basename) {
    Shader* baseShader = ShaderManager::loadFromFile(basename);
    return reinterpret_cast<Vk_Shader*>(baseShader);
}

Vk_Shader* Vk_ShaderManager::loadFromMemory(const String& name, const std::map<ShaderType, String*>& shaderDataMap) {
    Shader* baseShader = ShaderManager::loadFromMemory(name, shaderDataMap);
    return reinterpret_cast<Vk_Shader*>(baseShader);
}

Vk_Shader* Vk_ShaderManager::getShader(const String& name) {
    Shader* baseShader = ShaderManager::getShader(name);
    return reinterpret_cast<Vk_Shader*>(baseShader);
}

Vk_Shader* Vk_ShaderManager::getActiveShader() {
    return reinterpret_cast<Vk_Shader*>(m_activeShader);
}

std::unique_ptr<Shader> Vk_ShaderManager::createShader() {
    return std::make_unique<Vk_Shader>();
}

void Vk_ShaderManager::useShader(Shader* shader) {
    ENGINE_UNUSED(shader);
}

const String& Vk_ShaderManager::getShaderFolder() const {
    return sShaderFolder;
}

}  // namespace engine
