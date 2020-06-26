#include <Core/Main.hpp>

#include "Vk_Plugin.hpp"

#include <memory>

namespace engine {

namespace {

std::unique_ptr<Vk_Plugin> sPlugin;

}  // namespace

extern "C" void VULKAN_PLUGIN_API StartPlugin(void) {
    sPlugin = std::make_unique<Vk_Plugin>();
    Main::GetInstance().installPlugin(sPlugin.get());
}

extern "C" void VULKAN_PLUGIN_API StopPlugin(void) {
    Main::GetInstance().uninstallPlugin(sPlugin.get());
    sPlugin.reset();
}

}  // namespace engine
