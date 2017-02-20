#include <Renderer/Renderer.hpp>

#include <SDL.h>

namespace engine {

Renderer::Renderer() : m_render_window(nullptr) {}

Renderer::~Renderer() {}

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
