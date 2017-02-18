#pragma once

#include <Util/Prerequisites.hpp>

#include <Renderer/Shader.hpp>
#include <Renderer/Texture2D.hpp>
#include <Renderer/RenderWindow.hpp>
#include <Graphics/Color.hpp>

namespace engine {

class ENGINE_API Renderer {
public:
    Renderer();

    virtual ~Renderer();

    virtual bool Initialize() = 0;

    virtual void Shutdown() = 0;

    virtual void AdvanceFrame();

    virtual Shader* CreateShader() = 0; // TMP?

    virtual Texture2D* CreateTexture2D() = 0; // TMP?

    RenderWindow& GetRenderWindow();

    RenderWindow* GetRenderWindowPtr();

protected:
    RenderWindow* render_window_;
};

}  // namespace engine
