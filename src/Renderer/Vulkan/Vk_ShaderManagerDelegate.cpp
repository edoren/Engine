#include "Vk_ShaderManagerDelegate.hpp"
#include "Vk_Shader.hpp"

namespace engine {

namespace {

const String sTag("Vk_ShaderManagerDelegate");

const String sShaderFolder("spirv");

}  // namespace

Vk_ShaderManagerDelegate::Vk_ShaderManagerDelegate() {}

Vk_ShaderManagerDelegate::~Vk_ShaderManagerDelegate() {}

Shader* Vk_ShaderManagerDelegate::CreateShader() {
    return new Vk_Shader();
}

void Vk_ShaderManagerDelegate::DeleteShader(Shader* shader) {
    delete shader;
}

void Vk_ShaderManagerDelegate::SetActiveShader(Shader* shader) {
    ENGINE_UNUSED(shader);
}

const String& Vk_ShaderManagerDelegate::GetShaderFolder() const {
    return sShaderFolder;
}

}  // namespace engine
