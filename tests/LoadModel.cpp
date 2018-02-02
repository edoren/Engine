#include <Core/Main.hpp>
#include <Graphics/3D/Camera.hpp>
#include <Input/InputManager.hpp>
#include <Renderer/Model.hpp>
#include <Renderer/RenderWindow.hpp>
#include <Renderer/Renderer.hpp>
#include <Renderer/Shader.hpp>
#include <Renderer/ShaderManager.hpp>
#include <System/Stopwatch.hpp>
#include <System/StringFormat.hpp>

#include <SDL.h>  // TMP

using namespace engine;

#ifdef ENGINE_DEBUG
#define VULKAN_PLUGIN_NAME "vulkan-plugin-d"
#define OPENGL_PLUGIN_NAME "opengl-plugin-d"
#else
#define VULKAN_PLUGIN_NAME "vulkan-plugin"
#define OPENGL_PLUGIN_NAME "opengl-plugin"
#endif

struct Vertex {
    Vertex(const math::vec3& position, const math::vec3& normal,
           const math::vec2& tex_coords)
          : position(position), normal(normal), tex_coords(tex_coords) {}
    math::vec3 position;
    math::vec3 normal;
    math::vec2 tex_coords;
};

#include <iostream>

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
    ShaderManager& shader_manager = ShaderManager::GetInstance();
    Renderer& render = engine.GetActiveRenderer();

    math::ivec2 window_size = {800, 600};
    RenderWindow& window = render.GetRenderWindow();

    shader_manager.LoadFromFile("model");

    bool ok = window.Create("My Game Test", window_size);

    if (!ok) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Init failed, exiting!");
        return 1;
    }

    math::mat4 projection_matrix;
    math::mat4 view_matrix;
    math::mat4 model_matrix;

    Model* character = new Model("LinkOcarina/YoungLinkEquipped.obj");

    Mouse& mouse = input.GetMouse();
    mouse.SetRelativeMouseMode(true);
    mouse.HideCursor();

    Camera camera({10, 10, 10});
    camera.LookAt({0, 0, 0});
    float camera_speed = 2.5f;
    float mouse_sensivity = 0.1f;

    window.SetActiveCamera(&camera);

    Stopwatch timer;
    float delta_time;

    float angle = 0;

    timer.Start();
    while (!input.exit_requested()) {
        delta_time = timer.GetElapsedTime().AsSeconds();
        timer.Restart();

        window.Clear(Color::BLACK);

        // Camera movement
        const math::vec3& camera_front = camera.GetFrontVector();
        math::vec3 camera_forward(camera_front.x, 0, camera_front.z);
        camera_forward = math::Normalize(camera_forward);

        // Camera mouse movement
        math::vec2 mouse_delta(mouse.pointer.mousedelta);
        camera.Rotate(mouse_delta * mouse_sensivity);

        // Camera key movements
        float speed = camera_speed * delta_time;
        if (input.GetButton(SDLK_w).IsDown())
            camera.Move(speed * camera_forward);
        if (input.GetButton(SDLK_s).IsDown())
            camera.Move(speed * -camera_forward);
        if (input.GetButton(SDLK_d).IsDown())
            camera.Move(speed * camera.GetRightVector());
        if (input.GetButton(SDLK_a).IsDown())
            camera.Move(speed * -camera.GetRightVector());

        if (input.GetButton(SDLK_SPACE).IsDown())
            camera.Move(speed * Camera::WORLD_UP);
        if (input.GetButton(SDLK_LSHIFT).IsDown())
            camera.Move(speed * -Camera::WORLD_UP);

        window_size = window.GetSize();

        float aspect_ratio = window_size.x / static_cast<float>(window_size.y);
        angle += math::Radians(delta_time * 90.f);

        view_matrix = camera.GetViewMatrix();
        projection_matrix =
            math::Perspective(math::Radians(45.f), aspect_ratio, 0.1f, 100.0f);

        shader_manager.SetActiveShader("model");

        window.Draw(*character);

        input.AdvanceFrame();
        render.AdvanceFrame();
        // render.DepthTest(false);
    }

    delete character;

    engine.Shutdown();

    return 0;
}
