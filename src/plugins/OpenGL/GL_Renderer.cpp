#include "GL_Renderer.hpp"
#include "GL_Dependencies.hpp"
#include "GL_RenderWindow.hpp"
#include "GL_ShaderManager.hpp"
#include "GL_TextureManager.hpp"

namespace engine::plugin::opengl {

namespace {

String sRendererName("OpenGL");

}  // namespace

GL_Renderer::GL_Renderer() = default;

GL_Renderer::~GL_Renderer() {
    shutdown();
}

bool GL_Renderer::initialize() {
    bool ok = Renderer::initialize();
    if (ok) {
        m_renderWindow = std::make_unique<GL_RenderWindow>();
        m_shaderManager = std::make_unique<GL_ShaderManager>();
        m_textureManager = std::make_unique<GL_TextureManager>();
    }
    return ok;
}

void GL_Renderer::shutdown() {
    m_textureManager.reset();
    m_shaderManager.reset();
    m_renderWindow.reset();
    Renderer::shutdown();
}

void GL_Renderer::advanceFrame() {
    Renderer::advanceFrame();
    // TODO: User enable depth test
    if (!glIsEnabled(GL_DEPTH_TEST)) {
        GL_CALL(glEnable(GL_DEPTH_TEST));
    }
}

const String& GL_Renderer::getName() const {
    return sRendererName;
}

}  // namespace engine::plugin::opengl
