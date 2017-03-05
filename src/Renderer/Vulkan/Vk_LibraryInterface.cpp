#include <Core/Main.hpp>

#include "Vk_Plugin.hpp"

namespace engine {

namespace {
Vk_Plugin* s_plugin;
}  // namespace

extern "C" void VULKAN_PLUGIN_API StartPlugin(void) {
    s_plugin = new Vk_Plugin();
    Main::GetInstance().InstallPlugin(s_plugin);
}

extern "C" void VULKAN_PLUGIN_API StopPlugin(void) {
    Main::GetInstance().UninstallPlugin(s_plugin);
    delete s_plugin;
}

}  // namespace engine
