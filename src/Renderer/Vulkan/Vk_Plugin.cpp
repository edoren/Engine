#include "Vk_Plugin.hpp"

#include <Core/Main.hpp>

namespace engine {

namespace {
const String s_plugin_name("Vulkan");
}  // namespace

Vk_Plugin::Vk_Plugin() : m_renderer(nullptr) {}

const String& Vk_Plugin::GetName() const {
    return s_plugin_name;
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
