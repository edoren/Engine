#pragma once

#include <Graphics/Image.hpp>
#include <Renderer/Texture2D.hpp>
#include <Util/Prerequisites.hpp>

#include "GL_Dependencies.hpp"

namespace engine {

class ENGINE_API GL_Texture2D : public Texture2D {
public:
    GL_Texture2D();
    GL_Texture2D(GL_Texture2D&& other);
    ~GL_Texture2D();

    GL_Texture2D& operator=(GL_Texture2D&& other);

    bool LoadFromImage(const Image& img);

    void Use();

private:
    uint32 texture_;
};

}  // namespace engine
