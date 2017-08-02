#include <Core/Main.hpp>

#include "Vk_Plugin.hpp"

namespace engine {

namespace {

Vk_Plugin* sPlugin;

}  // namespace

extern "C" void VULKAN_PLUGIN_API StartPlugin(void) {
    sPlugin = new Vk_Plugin();
    Main::GetInstance().InstallPlugin(sPlugin);
}

extern "C" void VULKAN_PLUGIN_API StopPlugin(void) {
    Main::GetInstance().UninstallPlugin(sPlugin);
    delete sPlugin;
}

}  // namespace engine
