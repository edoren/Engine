#include "Vk_Renderer.hpp"
// #include "Vk_Shader.hpp"
// #include "Vk_Texture2D.hpp"

namespace engine {

Vk_Renderer::Vk_Renderer() {}

Vk_Renderer::~Vk_Renderer() {
    Shutdown();
}

bool Vk_Renderer::Initialize() {
    bool ok = Renderer::Initialize();
    if (ok) {
        m_context = new Vk_Context();
        ok = ok && m_context->Initialize();
        m_render_window = new Vk_RenderWindow();
    }
    return ok;
}

void Vk_Renderer::Shutdown() {
    delete m_render_window;
    m_render_window = nullptr;
    delete m_context;
    m_context = nullptr;
    Renderer::Shutdown();
}

void Vk_Renderer::AdvanceFrame() {
    Renderer::AdvanceFrame();
    // TODO: User enable depth test
    // Vk_CALL(glEnable(Vk_DEPTH_TEST));
}

Shader* Vk_Renderer::CreateShader() {
    return nullptr;
}

Texture2D* Vk_Renderer::CreateTexture2D() {
    return nullptr;
}

// Mesh* Vk_Renderer::CreateMesh() {
//     return nullptr;
// }

}  // namespace engine
