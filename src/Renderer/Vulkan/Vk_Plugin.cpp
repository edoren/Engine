#include "Vk_Plugin.hpp"
#include "Vk_Renderer.hpp"
#include "Vk_ShaderManagerDelegate.hpp"
#include "Vk_TextureManagerDelegate.hpp"

#include <Core/Main.hpp>
#include <System/String.hpp>

namespace engine {

namespace {

const String sPluginName("Vulkan");

}  // namespace

Vk_Plugin::Vk_Plugin() : m_renderer(nullptr) {}

const String& Vk_Plugin::GetName() const {
    return sPluginName;
}

void Vk_Plugin::Install() {
    m_renderer = new Vk_Renderer();
    Main::GetInstance().AddRenderer(m_renderer);
    m_shader_manager_delegate = new Vk_ShaderManagerDelegate();
    ShaderManager::GetInstance().SetDelegate(m_shader_manager_delegate);
    m_texture_manager_delegate = new Vk_TextureManagerDelegate();
    TextureManager::GetInstance().SetDelegate(m_texture_manager_delegate);
}

void Vk_Plugin::Initialize() {}

void Vk_Plugin::Shutdown() {}

void Vk_Plugin::Uninstall() {
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
