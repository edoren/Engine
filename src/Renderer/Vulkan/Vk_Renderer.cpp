#include "Vk_Renderer.hpp"
#include "Vk_RendererFactory.hpp"
#include "Vk_ShaderManager.hpp"
#include "Vk_TextureManager.hpp"

namespace engine {

namespace {

String sRendererName("Vulkan");

}  // namespace

Vk_Renderer::Vk_Renderer() {}

Vk_Renderer::~Vk_Renderer() {
    Shutdown();
}

bool Vk_Renderer::Initialize() {
    bool ok = Renderer::Initialize();
    if (ok) {
        m_context = std::make_unique<Vk_Context>();
        ok = ok && m_context->Initialize();
        m_render_window = std::make_unique<Vk_RenderWindow>();
        m_renderer_factory = std::make_unique<Vk_RendererFactory>();
        m_shader_manager = std::make_unique<Vk_ShaderManager>();
        m_texture_manager = std::make_unique<Vk_TextureManager>();

        // TODO: Remove this
        m_shader_manager->LoadFromFile("model");
    }
    return ok;
}

void Vk_Renderer::Shutdown() {
    m_texture_manager.reset();
    m_shader_manager.reset();
    m_renderer_factory.reset();
    m_render_window.reset();
    m_context.reset();
    Renderer::Shutdown();
}

void Vk_Renderer::AdvanceFrame() {
    Renderer::AdvanceFrame();
    // TODO: User enable depth test
    // Vk_CALL(glEnable(Vk_DEPTH_TEST));
}

const String& Vk_Renderer::GetName() const {
    return sRendererName;
}

}  // namespace engine
