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
    sDerivedInstance = reinterpret_cast<Vk_ShaderManager*>(&base_instance);}

Vk_ShaderManager::~Vk_ShaderManager() {
    for (auto shader_pair : m_shaders) {
        DeleteShader(shader_pair.second);
    }
    m_shaders.clear();
    sDerivedInstance = nullptr;
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
