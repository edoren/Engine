#pragma once

#include <Graphics/Image.hpp>
#include <Renderer/Texture2D.hpp>

#include "GL_Config.hpp"
#include "GL_Dependencies.hpp"

namespace engine::plugin::opengl {

class OPENGL_PLUGIN_API GL_Texture2D : public Texture2D {
public:
    GL_Texture2D();

    ~GL_Texture2D() override;

    bool loadFromImage(const Image& img) override;

    void use() override;

private:
    uint32 m_texture;
};

}  // namespace engine::plugin::opengl
