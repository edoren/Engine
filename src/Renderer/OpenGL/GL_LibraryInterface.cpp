
#include <Core/Main.hpp>

#include "GL_Plugin.hpp"

namespace engine {

namespace {

GL_Plugin* sPlugin;

}  // namespace

extern "C" void OPENGL_PLUGIN_API StartPlugin(void) {
    sPlugin = new GL_Plugin();
    Main::GetInstance().InstallPlugin(sPlugin);
}

extern "C" void OPENGL_PLUGIN_API StopPlugin(void) {
    Main::GetInstance().UninstallPlugin(sPlugin);
    delete sPlugin;
}

}  // namespace engine
