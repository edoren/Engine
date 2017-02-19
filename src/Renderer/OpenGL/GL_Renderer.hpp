#pragma once

#include <Renderer/Renderer.hpp>

#include "GL_Config.hpp"
#include "GL_Dependencies.hpp"
#include "GL_RenderWindow.hpp"

namespace engine {

class OPENGL_PLUGIN_API GL_Renderer : public Renderer {
public:
    GL_Renderer();

    virtual ~GL_Renderer();

    virtual bool Initialize();

    virtual void Shutdown();

    virtual void AdvanceFrame();

    virtual Shader* CreateShader();  // TMP?

    virtual Texture2D* CreateTexture2D();  // TMP?
};

}  // namespace engine
