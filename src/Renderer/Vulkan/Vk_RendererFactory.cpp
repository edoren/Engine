#include "Vk_RendererFactory.hpp"
#include "Vk_Shader.hpp"
#include "Vk_Mesh.hpp"
#include "Vk_Texture2D.hpp"

namespace engine {

Vk_RendererFactory::Vk_RendererFactory() {}

Vk_RendererFactory::~Vk_RendererFactory() {}

Shader* Vk_RendererFactory::CreateShader() {
    return nullptr;
}

Texture2D* Vk_RendererFactory::CreateTexture2D() {
    return nullptr;
}

Mesh* Vk_RendererFactory::CreateMesh() {
    return new Vk_Mesh();
}

}  // namespace engine
