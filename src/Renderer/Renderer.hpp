#pragma once

#include <Util/Prerequisites.hpp>

#include <Renderer/RenderWindow.hpp>

namespace engine {

class Shader;
class Texture2D;
class Mesh;

class ENGINE_API Renderer {
public:
    Renderer();

    virtual ~Renderer();

    virtual bool Initialize();

    virtual void Shutdown();

    virtual void AdvanceFrame();

    // TODO: Change this for a factory
    virtual Shader* CreateShader() = 0;
    virtual Texture2D* CreateTexture2D() = 0;
    // virtual Mesh* CreateMesh() = 0;

    RenderWindow& GetRenderWindow();

    RenderWindow* GetRenderWindowPtr();

protected:
    RenderWindow* m_render_window;
};

}  // namespace engine
