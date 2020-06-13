#include <Renderer/ModelManager.hpp>
#include <Renderer/RenderWindow.hpp>
#include <Renderer/Renderer.hpp>
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
        m_shader_manager(nullptr),
        m_texture_manager(nullptr),
        m_model_manager(nullptr) {}

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
    if (!m_render_window) {
        return;
    }
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

}  // namespace engine
