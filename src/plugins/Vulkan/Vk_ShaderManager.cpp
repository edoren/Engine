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
    ShaderManager& base_instance = ShaderManager::GetInstance();
    sDerivedInstance = reinterpret_cast<Vk_ShaderManager*>(&base_instance);
}

Vk_ShaderManager::~Vk_ShaderManager() {
    sDerivedInstance = nullptr;
}

Vk_Shader* Vk_ShaderManager::loadFromFile(const String& basename) {
    Shader* base_shader = ShaderManager::loadFromFile(basename);
    return reinterpret_cast<Vk_Shader*>(base_shader);
}

Vk_Shader* Vk_ShaderManager::loadFromMemory(const String& name, const std::map<ShaderType, String*>& shaderDataMap) {
    Shader* base_shader = ShaderManager::loadFromMemory(name, shaderDataMap);
    return reinterpret_cast<Vk_Shader*>(base_shader);
}

Vk_Shader* Vk_ShaderManager::getShader(const String& name) {
    Shader* base_shader = ShaderManager::getShader(name);
    return reinterpret_cast<Vk_Shader*>(base_shader);
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
