#include "Vk_Renderer.hpp"
// #include "Vk_Shader.hpp"
// #include "Vk_Texture2D.hpp"

namespace engine {

Vk_Renderer::Vk_Renderer() {}

Vk_Renderer::~Vk_Renderer() {
    Shutdown();
}

bool Vk_Renderer::Initialize() {
    m_core.Initialize();
    m_render_window = new Vk_RenderWindow(&m_core);
    return m_render_window != nullptr;
}

void Vk_Renderer::Shutdown() {
    delete m_render_window;
    m_render_window = nullptr;
    m_core.Shutdown();
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

}  // namespace engine
