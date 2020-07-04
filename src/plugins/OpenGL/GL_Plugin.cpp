#include "GL_Plugin.hpp"
#include "GL_Renderer.hpp"

#include <Core/Main.hpp>

namespace engine::plugin::opengl {

namespace {

const String sPluginName("OpenGL");

}  // namespace

GL_Plugin::GL_Plugin() : m_renderer(nullptr), m_shaderManagerDelegate(nullptr), m_textureManagerDelegate(nullptr) {}

const String& GL_Plugin::getName() const {
    return sPluginName;
}

void GL_Plugin::install() {
    auto renderer = std::make_unique<GL_Renderer>();
    m_renderer = renderer.get();
    Main::GetInstance().addRenderer(std::move(renderer));
}

void GL_Plugin::initialize() {}

void GL_Plugin::shutdown() {}

void GL_Plugin::uninstall() {
    m_renderer = nullptr;
}

}  // namespace engine::plugin::opengl
