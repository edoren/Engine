#include "GL_Renderer.hpp"
#include "GL_Dependencies.hpp"
#include "GL_RenderWindow.hpp"
#include "GL_ShaderManager.hpp"
#include "GL_TextureManager.hpp"

namespace engine {

namespace {

String sRendererName("OpenGL");

}  // namespace

GL_Renderer::GL_Renderer() {}

GL_Renderer::~GL_Renderer() {
    Shutdown();
}

bool GL_Renderer::Initialize() {
    bool ok = Renderer::Initialize();
    if (ok) {
        m_render_window = std::make_unique<GL_RenderWindow>();
        m_shader_manager = std::make_unique<GL_ShaderManager>();
        m_texture_manager = std::make_unique<GL_TextureManager>();
    }
    return ok;
}

void GL_Renderer::Shutdown() {
    m_texture_manager.reset();
    m_shader_manager.reset();
    m_render_window.reset();
    Renderer::Shutdown();
}

void GL_Renderer::AdvanceFrame() {
    Renderer::AdvanceFrame();
    // TODO: User enable depth test
    if (!glIsEnabled(GL_DEPTH_TEST)) {
        GL_CALL(glEnable(GL_DEPTH_TEST));
    }
}

const String& GL_Renderer::GetName() const {
    return sRendererName;
}

}  // namespace engine
