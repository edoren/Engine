#include <Renderer/RenderWindow.hpp>
#include <Renderer/Renderer.hpp>
#include <Renderer/RendererFactory.hpp>
#include <Renderer/ShaderManager.hpp>
#include <Renderer/TextureManager.hpp>
#include <System/LogManager.hpp>
#include <System/String.hpp>
#include <System/StringFormat.hpp>

#include <SDL2.h>

namespace engine {

namespace {

const String sTag("Renderer");

}  // namespace

Renderer::Renderer()
      : m_render_window(nullptr),
        m_renderer_factory(nullptr),
        m_shader_manager(nullptr),
        m_texture_manager(nullptr) {}

Renderer::~Renderer() {}

bool Renderer::Initialize() {
    int code = SDL_InitSubSystem(SDL_INIT_VIDEO);
    LogInfo(sTag, "Current SDL video driver: {}"_format(SDL_GetCurrentVideoDriver()));
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

std::unique_ptr<RenderWindow>& Renderer::GetRenderWindowPtr() {
    return m_render_window;
}

RendererFactory& Renderer::GetRendererFactory() {
    assert(m_renderer_factory);
    return *m_renderer_factory;
}

std::unique_ptr<RendererFactory>& Renderer::GetRendererFactoryPtr() {
    return m_renderer_factory;
}

ShaderManager& Renderer::GetShaderManager() {
    assert(m_shader_manager);
    return *m_shader_manager;
}

std::unique_ptr<ShaderManager>& Renderer::GetShaderManagerPtr() {
    return m_shader_manager;
}

TextureManager& Renderer::GetTextureManager() {
    assert(m_texture_manager);
    return *m_texture_manager;
}

std::unique_ptr<TextureManager>& Renderer::GetTextureManagerPtr() {
    return m_texture_manager;
}

}  // namespace engine
