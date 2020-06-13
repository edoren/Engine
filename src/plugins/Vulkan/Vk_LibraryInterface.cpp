#include <Core/Main.hpp>

#include "Vk_Plugin.hpp"

namespace engine {

namespace {

std::unique_ptr<Vk_Plugin> sPlugin;

}  // namespace

extern "C" void VULKAN_PLUGIN_API StartPlugin(void) {
    sPlugin = std::make_unique<Vk_Plugin>();
    Main::GetInstance().InstallPlugin(sPlugin.get());
}

extern "C" void VULKAN_PLUGIN_API StopPlugin(void) {
    Main::GetInstance().UninstallPlugin(sPlugin.get());
    sPlugin.reset();
}

}  // namespace engine
