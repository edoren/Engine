#include "Vk_Plugin.hpp"
#include "Vk_Renderer.hpp"

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
}

void Vk_Plugin::Initialize() {}

void Vk_Plugin::Shutdown() {}

void Vk_Plugin::Uninstall() {
    delete m_renderer;
    m_renderer = nullptr;
}

}  // namespace engine
