#include <Core/Main.hpp>

#include "GL_Plugin.hpp"

namespace engine {

namespace {

std::unique_ptr<GL_Plugin> sPlugin;

}  // namespace

extern "C" void OPENGL_PLUGIN_API StartPlugin(void) {
    sPlugin = std::make_unique<GL_Plugin>();
    Main::GetInstance().installPlugin(sPlugin.get());
}

extern "C" void OPENGL_PLUGIN_API StopPlugin(void) {
    Main::GetInstance().uninstallPlugin(sPlugin.get());
    sPlugin.reset();
}

}  // namespace engine
