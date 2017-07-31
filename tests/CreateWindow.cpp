#include <Core/Main.hpp>
#include <Input/InputManager.hpp>
#include <Renderer/Renderer.hpp>
#include <System/StringFormat.hpp>

// TODO: Remove this later
#include <SDL.h>

#include <iostream>

#ifdef ENGINE_DEBUG
#define VULKAN_PLUGIN_NAME "vulkan-plugin-d"
#define OPENGL_PLUGIN_NAME "opengl-plugin-d"
#else
#define VULKAN_PLUGIN_NAME "vulkan-plugin"
#define OPENGL_PLUGIN_NAME "opengl-plugin"
#endif

using namespace engine;

int main(int argc, char* argv[]) {
    Main engine(argc, argv);

#if PLATFORM_TYPE_IS(PLATFORM_TYPE_MOBILE)
    String plugin = "vulkan";
#else
    String plugin = (argc == 2) ? argv[1] : "";
#endif
    if (plugin == "vulkan") {
        engine.LoadPlugin(VULKAN_PLUGIN_NAME);
    } else if (plugin == "opengl") {
        engine.LoadPlugin(OPENGL_PLUGIN_NAME);
    } else {
        String usage_msg = "Usage: {} [vulkan|opengl]"_format(argv[0]);
        std::cout << usage_msg.ToUtf8() << std::endl;
        return 0;
    }

    engine.Initialize();

    InputManager& input = InputManager::GetInstance();
    Renderer& render = engine.GetActiveRenderer();

    math::ivec2 window_size = {800, 600};
    RenderWindow& window = render.GetRenderWindow();

    bool ok = window.Create("My Game Test", window_size);

    if (!ok) {
        LogError("main", "Init failed, exiting!");
        return 1;
    }

    while (!input.exit_requested()) {
        window.Clear(Color::BLACK);

        input.AdvanceFrame(&window_size);
        render.AdvanceFrame();
    }

    engine.Shutdown();

    return 0;
}
