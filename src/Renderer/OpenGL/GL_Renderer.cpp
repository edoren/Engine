#include <Renderer/OpenGL/GL_Renderer.hpp>
#include <Renderer/OpenGL/GL_Utilities.hpp>

namespace engine {

GL_Renderer::GL_Renderer() {}

GL_Renderer::~GL_Renderer() {
    ShutDown();
}

bool GL_Renderer::Initialize() {
    render_window_ = new GL_RenderWindow();
    return render_window_ != nullptr;
}

void GL_Renderer::ShutDown() {
    delete render_window_;
}

void GL_Renderer::AdvanceFrame() {
    Renderer::AdvanceFrame();
    // TODO: User enable depth test
    GL_CALL(glEnable(GL_DEPTH_TEST));
}

}  // namespace engine
