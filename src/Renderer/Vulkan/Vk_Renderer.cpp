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
        m_context = new Vk_Context();
        ok = ok && m_context->Initialize();
        m_render_window = new Vk_RenderWindow();
        m_renderer_factory = new Vk_RendererFactory();
        m_shader_manager = new Vk_ShaderManager();
        m_texture_manager = new Vk_TextureManager();

        m_shader_manager->LoadFromFile("model");
    }
    return ok;
}

void Vk_Renderer::Shutdown() {
    delete m_texture_manager;
    m_texture_manager = nullptr;
    delete m_shader_manager;
    m_shader_manager = nullptr;
    delete m_renderer_factory;
    m_renderer_factory = nullptr;
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

const String& Vk_Renderer::GetName() const {
    return sRendererName;
}

}  // namespace engine
