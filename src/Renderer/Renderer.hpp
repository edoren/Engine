#pragma once

#include <Util/Prerequisites.hpp>

namespace engine {

class RendererFactory;
class RenderWindow;
class ShaderManager;
class String;
class TextureManager;

class ENGINE_API Renderer {
public:
    Renderer();

    virtual ~Renderer();

    virtual bool Initialize();

    virtual void Shutdown();

    virtual void AdvanceFrame();

    virtual const String& GetName() const = 0;

    RenderWindow& GetRenderWindow();
    std::unique_ptr<RenderWindow>& GetRenderWindowPtr();

    RendererFactory& GetRendererFactory();
    std::unique_ptr<RendererFactory>& GetRendererFactoryPtr();

    ShaderManager& GetShaderManager();
    std::unique_ptr<ShaderManager>& GetShaderManagerPtr();

    TextureManager& GetTextureManager();
    std::unique_ptr<TextureManager>& GetTextureManagerPtr();

protected:
    std::unique_ptr<RenderWindow> m_render_window;
    std::unique_ptr<RendererFactory> m_renderer_factory;
    std::unique_ptr<ShaderManager> m_shader_manager;
    std::unique_ptr<TextureManager> m_texture_manager;
};

}  // namespace engine
