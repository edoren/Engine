#pragma once

#include <Util/Prerequisites.hpp>

namespace engine {

class RenderWindow;
class RendererFactory;

class ENGINE_API Renderer {
public:
    Renderer();

    virtual ~Renderer();

    virtual bool Initialize();

    virtual void Shutdown();

    virtual void AdvanceFrame();

    RenderWindow& GetRenderWindow();
    RenderWindow* GetRenderWindowPtr();

    RendererFactory& GetRendererFactory();
    RendererFactory* GetRendererFactoryPtr();

protected:
    RenderWindow* m_render_window;
    RendererFactory* m_renderer_factory;
};

}  // namespace engine
