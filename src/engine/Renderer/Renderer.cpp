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

bool Renderer::initialize() {
    int code = SDL_InitSubSystem(SDL_INIT_VIDEO);
    LogInfo(sTag, "Current SDL video driver: {}"_format(SDL_GetCurrentVideoDriver()));
    return code == 0;
}

void Renderer::shutdown() {
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

void Renderer::advanceFrame() {
    if (!m_render_window) {
        return;
    }
    if (!m_render_window->isVisible()) {
        SDL_Delay(10);
    } else {
        m_render_window->swapBuffers();
    }
}

RenderWindow& Renderer::getRenderWindow() {
    assert(m_render_window);
    return *m_render_window;
}

std::unique_ptr<RenderWindow>& Renderer::getRenderWindowPtr() {
    return m_render_window;
}

}  // namespace engine
