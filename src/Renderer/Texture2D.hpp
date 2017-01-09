#pragma once

#include <Graphics/Image.hpp>
#include <Util/Precompiled.hpp>

namespace engine {

class Texture2D : NonCopyable {
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
