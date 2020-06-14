#include "Vk_Renderer.hpp"
#include "Vk_ModelManager.hpp"
#include "Vk_ShaderManager.hpp"
#include "Vk_TextureManager.hpp"

namespace engine {

namespace {

String sRendererName("Vulkan");

}  // namespace

Vk_Renderer::Vk_Renderer() {}

Vk_Renderer::~Vk_Renderer() {
    shutdown();
}

bool Vk_Renderer::initialize() {
    bool ok = Renderer::initialize();
    if (ok) {
        m_context = std::make_unique<Vk_Context>();
        ok = ok && m_context->initialize();
        m_render_window = std::make_unique<Vk_RenderWindow>();
        m_shader_manager = std::make_unique<Vk_ShaderManager>();
        m_texture_manager = std::make_unique<Vk_TextureManager>();
        m_model_manager = std::make_unique<Vk_ModelManager>();

        // TODO: Remove this
        m_shader_manager->loadFromFile("model");
    }
    return ok;
}

void Vk_Renderer::shutdown() {
    m_model_manager.reset();
    m_texture_manager.reset();
    m_shader_manager.reset();
    m_render_window.reset();
    m_context.reset();
    Renderer::shutdown();
}

void Vk_Renderer::advanceFrame() {
    Renderer::advanceFrame();
    // TODO: User enable depth test
    // Vk_CALL(glEnable(Vk_DEPTH_TEST));
}

const String& Vk_Renderer::getName() const {
    return sRendererName;
}

}  // namespace engine
