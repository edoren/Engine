#include <Renderer/RenderWindow.hpp>
#include <Renderer/Renderer.hpp>
#include <Renderer/RendererFactory.hpp>
#include <Renderer/ShaderManager.hpp>
#include <Renderer/TextureManager.hpp>

#include <SDL.h>

namespace engine {

Renderer::Renderer()
      : m_render_window(nullptr),
        m_renderer_factory(nullptr),
        m_shader_manager(nullptr),
        m_texture_manager(nullptr) {}

Renderer::~Renderer() {}

bool Renderer::Initialize() {
    int code = SDL_InitSubSystem(SDL_INIT_VIDEO);
    return code == 0;
}

void Renderer::Shutdown() {
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

void Renderer::AdvanceFrame() {
    if (!m_render_window) return;
    if (!m_render_window->IsVisible()) {
        SDL_Delay(10);
    } else {
        m_render_window->SwapBuffers();
    }
}

RenderWindow& Renderer::GetRenderWindow() {
    assert(m_render_window);
    return *m_render_window;
}

RenderWindow* Renderer::GetRenderWindowPtr() {
    return m_render_window;
}

RendererFactory& Renderer::GetRendererFactory() {
    assert(m_renderer_factory);
    return *m_renderer_factory;
}

RendererFactory* Renderer::GetRendererFactoryPtr() {
    return m_renderer_factory;
}

ShaderManager* Renderer::GetShaderManagerPtr() {
    return m_shader_manager;
}

ShaderManager& Renderer::GetShaderManager() {
    assert(m_shader_manager);
    return *m_shader_manager;
}

TextureManager* Renderer::GetTextureManagerPtr() {
    return m_texture_manager;
}

TextureManager& Renderer::GetTextureManager() {
    assert(m_texture_manager);
    return *m_texture_manager;
}

}  // namespace engine
