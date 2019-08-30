#pragma once

#include <Renderer/RendererFactory.hpp>

#include "GL_Config.hpp"

namespace engine {

class OPENGL_PLUGIN_API GL_RendererFactory : public RendererFactory {
public:
    GL_RendererFactory();

    ~GL_RendererFactory();

    Shader* CreateShader() override;

    Texture2D* CreateTexture2D() override;

    Mesh* CreateMesh() override;
};

}  // namespace engine
