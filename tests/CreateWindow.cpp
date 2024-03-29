#include <Core/App.hpp>
#include <Core/Main.hpp>

// TODO: Remove this later - Required for calling main
#include <SDL2/SDL_main.h>

#ifdef ENGINE_DEBUG
#define VULKAN_PLUGIN_NAME "vulkan-plugin-d"
#define OPENGL_PLUGIN_NAME "opengl-plugin-d"
#else
#define VULKAN_PLUGIN_NAME "vulkan-plugin"
#define OPENGL_PLUGIN_NAME "opengl-plugin"
#endif

using namespace engine;

class CreateWindowApp : public App {
public:
    CreateWindowApp() = default;

protected:
    bool initialize() override {
        return true;
    }

    void update() override {}

    void shutdown() override {}

    String getName() override{
        return "CreateWindow";
    }

    math::Vector2<int32> getWindowSize() override {
        return {800, 600};
    }
};

int main(int argc, char* argv[]) {
#if PLATFORM_TYPE_IS(PLATFORM_TYPE_MOBILE)
    String renderer = "vulkan";
#else
    String renderer = (argc == 2) ? argv[1] : "";
#endif

    String plugin;
    if (renderer == "vulkan") {
        plugin = VULKAN_PLUGIN_NAME;
    } else if (renderer == "opengl") {
        plugin = OPENGL_PLUGIN_NAME;
    } else {
        return 1;
    }

    CreateWindowApp app;

    Main engine(argc, argv);
    engine.loadPlugin(plugin);
    engine.initialize(&app);
    engine.run();
    engine.shutdown();

    return 0;
}
