#include <Renderer/Renderer.hpp>

#include <SDL.h>

namespace engine {

Renderer::Renderer() : render_window_(nullptr) {}

Renderer::~Renderer() {}

void Renderer::AdvanceFrame() {
    if (!render_window_) return;
    if (!render_window_->IsVisible()) {
        SDL_Delay(10);
    } else {
        render_window_->SwapBuffers();
    }
}

RenderWindow& Renderer::GetRenderWindow() {
    assert(render_window_);
    return *render_window_;
}

RenderWindow* Renderer::GetRenderWindowPtr() {
    return render_window_;
}

}  // namespace engine
