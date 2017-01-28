#include <Renderer/Renderer.hpp>

#include <Util/Precompiled.hpp>

#include <iostream>

namespace engine {

Renderer::Renderer() {
    SDL_InitSubSystem(SDL_INIT_VIDEO);
}

Renderer::~Renderer() {
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

void Renderer::AdvanceFrame() {
    if (!render_window_) return;
    if (!render_window_->IsVisible()) {
        std::cout << "NOT VISIBLE!" << std::endl;
        SDL_Delay(10);
    } else {
        render_window_->SwapBuffers();
    }
}

}  // namespace engine
