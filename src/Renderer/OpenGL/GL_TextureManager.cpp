#include "GL_TextureManager.hpp"
#include "GL_Texture2D.hpp"

namespace engine {

namespace {

const String sTag("GL_TextureManager");

}  // namespace

GL_TextureManager::GL_TextureManager() {}

GL_TextureManager::~GL_TextureManager() {
    for (auto texture_pair : m_textures) {
        DeleteTexture2D(texture_pair.second);
    }
    m_textures.clear();
}

Texture2D* GL_TextureManager::CreateTexture2D() {
    return new GL_Texture2D();
}

void GL_TextureManager::DeleteTexture2D(Texture2D* texture) {
    delete texture;
}

void GL_TextureManager::SetActiveTexture2D(Texture2D* texture) {
    GL_Texture2D* casted_texture = reinterpret_cast<GL_Texture2D*>(texture);
    casted_texture->Use();
}

}  // namespace engine
