#include "GL_Plugin.hpp"
#include "GL_Renderer.hpp"
#include "GL_ShaderManagerDelegate.hpp"
#include "GL_TextureManagerDelegate.hpp"

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
    m_shader_manager_delegate = new GL_ShaderManagerDelegate();
    ShaderManager::GetInstance().SetDelegate(m_shader_manager_delegate);
    m_texture_manager_delegate = new GL_TextureManagerDelegate();
    TextureManager::GetInstance().SetDelegate(m_texture_manager_delegate);
}

void GL_Plugin::Initialize() {}

void GL_Plugin::Shutdown() {}

void GL_Plugin::Uninstall() {
    TextureManager::GetInstance().SetDelegate(nullptr);
    delete m_texture_manager_delegate;
    m_texture_manager_delegate = nullptr;
    ShaderManager::GetInstance().SetDelegate(nullptr);
    delete m_shader_manager_delegate;
    m_shader_manager_delegate = nullptr;
    delete m_renderer;
    m_renderer = nullptr;
}

}  // namespace engine
