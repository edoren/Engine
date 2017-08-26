#include <Renderer/Renderer.hpp>

#include <Renderer/RenderWindow.hpp>
#include <Renderer/Shader.hpp>
#include <Renderer/Texture2D.hpp>

#include <SDL.h>

namespace engine {

Renderer::Renderer() : m_render_window(nullptr) {}

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

}  // namespace engine
