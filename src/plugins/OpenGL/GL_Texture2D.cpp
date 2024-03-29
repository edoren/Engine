#include "GL_Texture2D.hpp"
#include "GL_Utilities.hpp"

namespace engine::plugin::opengl {

GL_Texture2D::GL_Texture2D() : m_texture(0) {
    GL_CALL(glGenTextures(1, &m_texture));
}

GL_Texture2D::~GL_Texture2D() {
    if (m_texture) {
        GL_CALL(glDeleteTextures(1, &m_texture));
    }
}

bool GL_Texture2D::loadFromImage(const Image& img) {
    GL_CALL(glBindTexture(GL_TEXTURE_2D, m_texture));

    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));

    const math::uvec2& size = img.getSize();
    const byte* data = img.getData();

    GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data));
    GL_CALL(glGenerateMipmap(GL_TEXTURE_2D));

    GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));

    return true;
}

void GL_Texture2D::use() {
    GL_CALL(glBindTexture(GL_TEXTURE_2D, m_texture));
}

}  // namespace engine::plugin::opengl
