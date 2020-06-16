#include "Vk_Renderer.hpp"
#include "Vk_ModelManager.hpp"
#include "Vk_ShaderManager.hpp"
#include "Vk_TextureManager.hpp"

namespace engine {

namespace {

String sRendererName("Vulkan");

}  // namespace

Vk_Renderer::Vk_Renderer() = default;

Vk_Renderer::~Vk_Renderer() {
    shutdown();
}

bool Vk_Renderer::initialize() {
    bool ok = Renderer::initialize();
    if (ok) {
        m_context = std::make_unique<Vk_Context>();
        ok = ok && m_context->initialize();
        m_renderWindow = std::make_unique<Vk_RenderWindow>();
        m_shaderManager = std::make_unique<Vk_ShaderManager>();
        m_textureManager = std::make_unique<Vk_TextureManager>();
        m_modelManager = std::make_unique<Vk_ModelManager>();

        // TODO: Remove this
        m_shaderManager->loadFromFile("model");
    }
    return ok;
}

void Vk_Renderer::shutdown() {
    m_modelManager.reset();
    m_textureManager.reset();
    m_shaderManager.reset();
    m_renderWindow.reset();
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
