#include "Vk_TextureManagerDelegate.hpp"
// #include "Vk_Texture2D.hpp"

namespace engine {

namespace {

const String sTag("Vk_TextureManagerDelegate");

}  // namespace

Vk_TextureManagerDelegate::Vk_TextureManagerDelegate() {}

Vk_TextureManagerDelegate::~Vk_TextureManagerDelegate() {}

Texture2D* Vk_TextureManagerDelegate::CreateTexture2D() {
    return nullptr;
}

void Vk_TextureManagerDelegate::DeleteTexture2D(Texture2D* texture) {}

void Vk_TextureManagerDelegate::SetActiveTexture2D(Texture2D* texture) {
    ENGINE_UNUSED(texture);
}

}  // namespace engine
