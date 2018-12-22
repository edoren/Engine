#include <Core/App.hpp>
#include <Core/Main.hpp>
#include <Graphics/3D/Camera.hpp>
#include <Renderer/Model.hpp>
#include <Renderer/RenderWindow.hpp>
#include <Renderer/ShaderManager.hpp>
#include <Renderer/SceneManager.hpp>

// TODO: Remove this later - Required for calling main and Input enums
#include <SDL2/SDL.h>

#ifdef ENGINE_DEBUG
#define VULKAN_PLUGIN_NAME "vulkan-plugin-d"
#define OPENGL_PLUGIN_NAME "opengl-plugin-d"
#else
#define VULKAN_PLUGIN_NAME "vulkan-plugin"
#define OPENGL_PLUGIN_NAME "opengl-plugin"
#endif

using namespace engine;

struct Vertex {
    Vertex(const math::vec3& position, const math::vec3& normal,
           const math::vec2& tex_coords)
          : position(position), normal(normal), tex_coords(tex_coords) {}
    math::vec3 position;
    math::vec3 normal;
    math::vec2 tex_coords;
};

class LoadModelApp : public App {
public:
    LoadModelApp() : m_window_size(800, 600) {}

protected:
    bool Initialize() override {
        Main& engine = Main::GetInstance();

        m_input = InputManager::GetInstancePtr();
        m_shader_manager = ShaderManager::GetInstancePtr();
        m_render = engine.GetActiveRendererPtr();
        m_window = m_render->GetRenderWindowPtr();

        m_shader_manager->LoadFromFile("model");

        SceneManager* scene_manager = SceneManager::GetInstancePtr();
        if (scene_manager) {
            scene_manager->ChangeActiveScene("test1");
        }

        Mouse& mouse = m_input->GetMouse();
        mouse.SetRelativeMouseMode(true);
        mouse.HideCursor();

        m_camera = Camera({10, 10, 10});
        m_camera.LookAt({0, 0, 0});
        m_camera_speed = 2.5f;
        m_mouse_sensivity = 0.1f;

        m_window->SetActiveCamera(&m_camera);
        // m_window->SetFullScreen(true, true);

        return true;
    }

    void Update() override {
        // Camera movement
        const math::vec3& camera_front = m_camera.GetFrontVector();
        math::vec3 camera_forward(camera_front.x, 0, camera_front.z);
        camera_forward = math::Normalize(camera_forward);

        // Camera mouse movement
        Mouse& mouse = m_input->GetMouse();
        math::vec2 mouse_delta(mouse.pointer.mousedelta);
        m_camera.Rotate(mouse_delta * m_mouse_sensivity);

        // Camera key movements
        float delta_time = GetDeltaTime().AsSeconds();
        float speed = m_camera_speed * delta_time;
        if (m_input->GetButton(SDLK_w).IsDown())
            m_camera.Move(speed * camera_forward);
        if (m_input->GetButton(SDLK_s).IsDown())
            m_camera.Move(speed * -camera_forward);
        if (m_input->GetButton(SDLK_d).IsDown())
            m_camera.Move(speed * m_camera.GetRightVector());
        if (m_input->GetButton(SDLK_a).IsDown())
            m_camera.Move(speed * -m_camera.GetRightVector());

        if (m_input->GetButton(SDLK_SPACE).IsDown())
            m_camera.Move(speed * Camera::WORLD_UP);
        if (m_input->GetButton(SDLK_LSHIFT).IsDown())
            m_camera.Move(speed * -Camera::WORLD_UP);

        m_window_size = m_window->GetSize();

        m_shader_manager->SetActiveShader("model");
    }

    void Shutdown() override {
    }

    String GetName() override {
        return "LoadModel";
    }

    math::Vector2<int32> GetWindowSize() override {
        return m_window_size;
    }

private:
    math::ivec2 m_window_size;

    Camera m_camera;
    float m_camera_speed;
    float m_mouse_sensivity;

    InputManager* m_input;
    ShaderManager* m_shader_manager;
    Renderer* m_render;
    RenderWindow* m_window;
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

    LoadModelApp app;

    Main engine(argc, argv);
    engine.LoadPlugin(plugin);
    engine.Initialize(&app);
    engine.Run();
    engine.Shutdown();

    return 0;
}
