#pragma once

#include <Util/Prerequisites.hpp>

namespace engine {

class RendererFactory;
class RenderWindow;
class ShaderManager;
class TextureManager;

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

    ShaderManager& GetShaderManager();
    ShaderManager* GetShaderManagerPtr();

    TextureManager& GetTextureManager();
    TextureManager* GetTextureManagerPtr();

protected:
    RenderWindow* m_render_window;
    RendererFactory* m_renderer_factory;
    ShaderManager* m_shader_manager;
    TextureManager* m_texture_manager;
};

}  // namespace engine
