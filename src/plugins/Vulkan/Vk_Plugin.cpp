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
    auto renderer = std::make_unique<Vk_Renderer>();
    m_renderer = renderer.get();
    Main::GetInstance().AddRenderer(std::move(renderer));
}

void Vk_Plugin::Initialize() {}

void Vk_Plugin::Shutdown() {}

void Vk_Plugin::Uninstall() {
    m_renderer = nullptr;
}

}  // namespace engine
