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

Vk_ShaderManager::Vk_ShaderManager() : ShaderManager() {
    ShaderManager& base_instance = ShaderManager::GetInstance();
    sDerivedInstance = reinterpret_cast<Vk_ShaderManager*>(&base_instance);
}

Vk_ShaderManager::~Vk_ShaderManager() {
    for (auto shader_pair : m_shaders) {
        DeleteShader(shader_pair.second);
    }
    m_shaders.clear();
    sDerivedInstance = nullptr;
}

Vk_Shader* Vk_ShaderManager::LoadFromFile(const String& basename) {
    Shader* base_shader = ShaderManager::LoadFromFile(basename);
    return reinterpret_cast<Vk_Shader*>(base_shader);
}

Vk_Shader* Vk_ShaderManager::LoadFromMemory(const String& name, std::map<ShaderType, String*> shader_data_map) {
    Shader* base_shader = ShaderManager::LoadFromMemory(name, shader_data_map);
    return reinterpret_cast<Vk_Shader*>(base_shader);
}

Vk_Shader* Vk_ShaderManager::GetShader(const String& name) {
    Shader* base_shader = ShaderManager::GetShader(name);
    return reinterpret_cast<Vk_Shader*>(base_shader);
}

Vk_Shader* Vk_ShaderManager::GetActiveShader() {
    return reinterpret_cast<Vk_Shader*>(m_active_shader);
}

Shader* Vk_ShaderManager::CreateShader() {
    return new Vk_Shader();
}

void Vk_ShaderManager::DeleteShader(Shader* shader) {
    delete shader;
}

void Vk_ShaderManager::UseShader(Shader* shader) {
    ENGINE_UNUSED(shader);
}

const String& Vk_ShaderManager::GetShaderFolder() const {
    return sShaderFolder;
}

}  // namespace engine
