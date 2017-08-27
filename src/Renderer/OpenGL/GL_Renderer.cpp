#include "GL_Renderer.hpp"
#include "GL_Dependencies.hpp"
#include "GL_RenderWindow.hpp"
#include "GL_RendererFactory.hpp"

namespace engine {

GL_Renderer::GL_Renderer() {}

GL_Renderer::~GL_Renderer() {
    Shutdown();
}

bool GL_Renderer::Initialize() {
    bool ok = Renderer::Initialize();
    if (ok) {
        m_render_window = new GL_RenderWindow();
        m_renderer_factory = new GL_RendererFactory();
    }
    return ok;
}

void GL_Renderer::Shutdown() {
    delete m_renderer_factory;
    m_renderer_factory = nullptr;
    delete m_render_window;
    m_render_window = nullptr;
    Renderer::Shutdown();
}

void GL_Renderer::AdvanceFrame() {
    Renderer::AdvanceFrame();
    // TODO: User enable depth test
    GL_CALL(glEnable(GL_DEPTH_TEST));
}

}  // namespace engine
