#pragma once

#include <Renderer/OpenGL/GL_Utilities.hpp>
#include <Graphics/Image.hpp>

namespace engine {

class ENGINE_API Texture2D : NonCopyable {
public:
    Texture2D();
    Texture2D(Texture2D&& other);
    ~Texture2D();

    Texture2D& operator=(Texture2D&& other);

    bool LoadFromImage(const Image& img);

    void Use();

private:
    GLuint texture_;
};

}  // namespace engine
