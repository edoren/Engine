#include "GL_Texture2D.hpp"
#include "GL_Utilities.hpp"

namespace engine {

GL_Texture2D::GL_Texture2D() : texture_(0) {
    glGenTextures(1, &texture_);
}

GL_Texture2D::GL_Texture2D(GL_Texture2D&& other) : texture_(other.texture_) {
    other.texture_ = 0;
}

GL_Texture2D::~GL_Texture2D() {
    if (texture_) {
        glDeleteTextures(1, &texture_);
    }
}

GL_Texture2D& GL_Texture2D::operator=(GL_Texture2D&& other) {
    texture_ = other.texture_;
    other.texture_ = 0;
    return *this;
}

bool GL_Texture2D::LoadFromImage(const Image& img) {
    GL_CALL(glBindTexture(GL_TEXTURE_2D, texture_));

    GL_CALL(
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GL_CALL(
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));

    const math::uvec2& size = img.GetSize();
    const byte* data = img.GetData();

    GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA,
                         GL_UNSIGNED_BYTE, data));
    GL_CALL(glGenerateMipmap(GL_TEXTURE_2D));

    GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));

    return true;
}

void GL_Texture2D::Use() {
    GL_CALL(glBindTexture(GL_TEXTURE_2D, texture_));
}

}  // namespace engine
