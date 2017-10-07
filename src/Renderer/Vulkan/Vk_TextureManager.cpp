#include "Vk_TextureManager.hpp"
// #include "Vk_Texture2D.hpp"

namespace engine {

namespace {

const String sTag("Vk_TextureManager");

}  // namespace

Vk_TextureManager::Vk_TextureManager() {}

Vk_TextureManager::~Vk_TextureManager() {
    for (auto texture_pair : m_textures) {
        DeleteTexture2D(texture_pair.second);
    }
    m_textures.clear();
}

Texture2D* Vk_TextureManager::CreateTexture2D() {
    return nullptr;
}

void Vk_TextureManager::DeleteTexture2D(Texture2D* texture) {
    ENGINE_UNUSED(texture);
}

void Vk_TextureManager::SetActiveTexture2D(Texture2D* texture) {
    ENGINE_UNUSED(texture);
}

}  // namespace engine