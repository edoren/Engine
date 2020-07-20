#include "Vk_Plugin.hpp"
#include "Vk_Renderer.hpp"

#include <Core/Main.hpp>
#include <System/String.hpp>

namespace engine::plugin::vulkan {

namespace {

const StringView sPluginName("Vulkan");

}  // namespace

Vk_Plugin::Vk_Plugin() : m_renderer(nullptr) {}

const StringView& Vk_Plugin::getName() const {
    return sPluginName;
}

void Vk_Plugin::install() {
    auto renderer = std::make_unique<Vk_Renderer>();
    m_renderer = renderer.get();
    Main::GetInstance().addRenderer(std::move(renderer));
}

void Vk_Plugin::initialize() {}

void Vk_Plugin::shutdown() {}

void Vk_Plugin::uninstall() {
    m_renderer = nullptr;
}

}  // namespace engine::plugin::vulkan
