#pragma once

#include <Renderer/Renderer.hpp>

#include "GL_Config.hpp"
#include "GL_Dependencies.hpp"
#include "GL_RenderWindow.hpp"

namespace engine {

class OPENGL_PLUGIN_API GL_Renderer : public Renderer {
public:
    GL_Renderer();

    ~GL_Renderer();

    bool Initialize() override;

    void Shutdown() override;

    void AdvanceFrame() override;

    // TODO: Change this for a factory
    Shader* CreateShader() override;
    Texture2D* CreateTexture2D() override;
    // Mesh* CreateMesh() override;
};

}  // namespace engine
