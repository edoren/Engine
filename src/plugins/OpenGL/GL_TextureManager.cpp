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

GL_TextureManager::GL_TextureManager() {
    TextureManager& base_instance = TextureManager::GetInstance();
    sDerivedInstance = reinterpret_cast<GL_TextureManager*>(&base_instance);
}

GL_TextureManager::~GL_TextureManager() {
    sDerivedInstance = nullptr;
}

std::unique_ptr<Texture2D> GL_TextureManager::CreateTexture2D() {
    return std::make_unique<GL_Texture2D>();
}

void GL_TextureManager::UseTexture2D(Texture2D* texture) {
    GL_Texture2D* casted_texture = reinterpret_cast<GL_Texture2D*>(texture);
    casted_texture->Use();
}

}  // namespace engine
