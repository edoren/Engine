#include "Vk_ShaderManager.hpp"
#include "Vk_Shader.hpp"

namespace engine {

namespace {

const String sTag("Vk_ShaderManager");

const String sShaderFolder("spirv");

}  // namespace

Vk_ShaderManager::Vk_ShaderManager() {}

Vk_ShaderManager::~Vk_ShaderManager() {}

Shader* Vk_ShaderManager::CreateShader() {
    return new Vk_Shader();
}

void Vk_ShaderManager::DeleteShader(Shader* shader) {
    delete shader;
}

void Vk_ShaderManager::SetActiveShader(Shader* shader) {
    ENGINE_UNUSED(shader);
}

const String& Vk_ShaderManager::GetShaderFolder() const {
    return sShaderFolder;
}

}  // namespace engine
