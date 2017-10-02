#include "GL_TextureManagerDelegate.hpp"
#include "GL_Texture2D.hpp"

namespace engine {

namespace {

const String sTag("GL_TextureManagerDelegate");

}  // namespace

GL_TextureManagerDelegate::GL_TextureManagerDelegate() {}

GL_TextureManagerDelegate::~GL_TextureManagerDelegate() {}

Texture2D* GL_TextureManagerDelegate::CreateTexture2D() {
    return new GL_Texture2D();
}

void GL_TextureManagerDelegate::DeleteTexture2D(Texture2D* texture) {
    delete texture;
}

void GL_TextureManagerDelegate::SetActiveTexture2D(Texture2D* texture) {
    GL_Texture2D* casted_texture = reinterpret_cast<GL_Texture2D*>(texture);
    casted_texture->Use();
}

}  // namespace engine
