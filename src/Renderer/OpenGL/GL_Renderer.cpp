#include "GL_Renderer.hpp"
#include "GL_Shader.hpp"
#include "GL_Texture2D.hpp"
#include "GL_Utilities.hpp"

namespace engine {

GL_Renderer::GL_Renderer() {}

GL_Renderer::~GL_Renderer() {
    Shutdown();
}

bool GL_Renderer::Initialize() {
    m_render_window = new GL_RenderWindow();
    return m_render_window != nullptr;
}

void GL_Renderer::Shutdown() {
    delete m_render_window;
    m_render_window = nullptr;
}

void GL_Renderer::AdvanceFrame() {
    Renderer::AdvanceFrame();
    // TODO: User enable depth test
    GL_CALL(glEnable(GL_DEPTH_TEST));
}

Shader* GL_Renderer::CreateShader() {
    return new GL_Shader();
}

Texture2D* GL_Renderer::CreateTexture2D() {
    return new GL_Texture2D();
}

}  // namespace engine
