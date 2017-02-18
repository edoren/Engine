
#include <Core/Main.hpp>

#include "GL_Plugin.hpp"

namespace engine {

namespace {
GL_Plugin* s_plugin;
}  // namespace

extern "C" void ENGINE_API StartPlugin(void) {
    s_plugin = new GL_Plugin();
    Main::GetInstance().InstallPlugin(s_plugin);
}

extern "C" void ENGINE_API StopPlugin(void) {
    Main::GetInstance().UninstallPlugin(s_plugin);
    delete s_plugin;
}

}  // namespace engine
