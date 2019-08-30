#include "GL_Plugin.hpp"
#include "GL_Renderer.hpp"

#include <Core/Main.hpp>

namespace engine {

namespace {

const String sPluginName("OpenGL");

}  // namespace

GL_Plugin::GL_Plugin()
      : m_renderer(nullptr),
        m_shader_manager_delegate(nullptr),
        m_texture_manager_delegate(nullptr) {}

const String& GL_Plugin::GetName() const {
    return sPluginName;
}

void GL_Plugin::Install() {
    auto renderer = std::make_unique<GL_Renderer>();
    m_renderer = renderer.get();
    Main::GetInstance().AddRenderer(std::move(renderer));
}

void GL_Plugin::Initialize() {}

void GL_Plugin::Shutdown() {}

void GL_Plugin::Uninstall() {
    m_renderer = nullptr;
}

}  // namespace engine
