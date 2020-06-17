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
    TextureManager& baseInstance = TextureManager::GetInstance();
    sDerivedInstance = reinterpret_cast<GL_TextureManager*>(&baseInstance);
}

GL_TextureManager::~GL_TextureManager() {
    sDerivedInstance = nullptr;
}

std::unique_ptr<Texture2D> GL_TextureManager::createTexture2D() {
    return std::make_unique<GL_Texture2D>();
}

void GL_TextureManager::useTexture2D(Texture2D* texture) {
    auto* castedTexture = reinterpret_cast<GL_Texture2D*>(texture);
    castedTexture->use();
}

}  // namespace engine
