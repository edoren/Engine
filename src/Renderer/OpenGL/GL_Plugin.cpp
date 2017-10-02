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
    m_renderer = new GL_Renderer();
    Main::GetInstance().AddRenderer(m_renderer);
}

void GL_Plugin::Initialize() {}

void GL_Plugin::Shutdown() {}

void GL_Plugin::Uninstall() {
    delete m_renderer;
    m_renderer = nullptr;
}

}  // namespace engine
