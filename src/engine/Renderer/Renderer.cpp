#include <Renderer/ModelManager.hpp>
#include <Renderer/RenderWindow.hpp>
#include <Renderer/Renderer.hpp>
#include <Renderer/ShaderManager.hpp>
#include <Renderer/TextureManager.hpp>
#include <System/LogManager.hpp>
#include <System/String.hpp>
#include <System/StringFormat.hpp>

#include <SDL2.h>

#include <memory>

namespace engine {

namespace {

const String sTag("Renderer");

}  // namespace

Renderer::Renderer()
      : m_renderWindow(nullptr),
        m_shaderManager(nullptr),
        m_textureManager(nullptr),
        m_modelManager(nullptr) {}

Renderer::~Renderer() = default;

bool Renderer::initialize() {
    int code = SDL_InitSubSystem(SDL_INIT_VIDEO);
    LogInfo(sTag, "Current SDL video driver: {}"_format(SDL_GetCurrentVideoDriver()));
    return code == 0;
}

void Renderer::shutdown() {
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

void Renderer::advanceFrame() {
    if (!m_renderWindow) {
        return;
    }
    if (!m_renderWindow->isVisible()) {
        SDL_Delay(10);
    } else {
        m_renderWindow->swapBuffers();
    }
}

RenderWindow& Renderer::getRenderWindow() {
    assert(m_renderWindow);
    return *m_renderWindow;
}

std::unique_ptr<RenderWindow>& Renderer::getRenderWindowPtr() {
    return m_renderWindow;
}

}  // namespace engine
