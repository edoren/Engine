#include "GL_TextureManager.hpp"
#include "GL_Texture2D.hpp"

namespace engine {

namespace {

const String sTag("GL_TextureManager");

GL_TextureManager* sDerivedInstance = nullptr;

}  // namespace

GL_TextureManager& GL_TextureManager::GetInstance() {
    assert(sDerivedInstance);
    return (*sDerivedInstance);
}

GL_TextureManager* GL_TextureManager::GetInstancePtr() {
    return sDerivedInstance;
}

GL_TextureManager::GL_TextureManager() : TextureManager() {
    TextureManager& base_instance = TextureManager::GetInstance();
    sDerivedInstance = reinterpret_cast<GL_TextureManager*>(&base_instance);
}

GL_TextureManager::~GL_TextureManager() {
    for (auto texture_pair : m_textures) {
        DeleteTexture2D(texture_pair.second);
    }
    m_textures.clear();
    sDerivedInstance = nullptr;
}

Texture2D* GL_TextureManager::CreateTexture2D() {
    return new GL_Texture2D();
}

void GL_TextureManager::DeleteTexture2D(Texture2D* texture) {
    delete texture;
}

void GL_TextureManager::UseTexture2D(Texture2D* texture) {
    GL_Texture2D* casted_texture = reinterpret_cast<GL_Texture2D*>(texture);
    casted_texture->Use();
}

}  // namespace engine
