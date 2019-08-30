#pragma once

#include <Graphics/Image.hpp>
#include <Renderer/Texture2D.hpp>

#include "GL_Config.hpp"
#include "GL_Dependencies.hpp"

namespace engine {

class OPENGL_PLUGIN_API GL_Texture2D : public Texture2D {
public:
    GL_Texture2D();
    GL_Texture2D(GL_Texture2D&& other);
    virtual ~GL_Texture2D();

    virtual GL_Texture2D& operator=(GL_Texture2D&& other);

    virtual bool LoadFromImage(const Image& img);

    virtual void Use();

private:
    uint32 m_texture;
};

}  // namespace engine
