#include <Renderer/Renderer.hpp>

#include <Util/Precompiled.hpp>

#include <iostream>

namespace engine {

Renderer::Renderer() : render_window_(nullptr) {
    int result = SDL_InitSubSystem(SDL_INIT_VIDEO);
}

Renderer::~Renderer() {
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

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
