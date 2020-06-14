#include <Core/App.hpp>
#include <Core/Main.hpp>
#include <Graphics/3D/Camera.hpp>
#include <Renderer/Model.hpp>
#include <Renderer/RenderWindow.hpp>
#include <Renderer/SceneManager.hpp>
#include <Renderer/ShaderManager.hpp>

// TODO: Remove this later - Required for calling main and Input enums
#include <SDL2/SDL.h>

#ifdef ENGINE_DEBUG
#define VULKAN_PLUGIN_NAME "vulkan-plugin-d"
#define OPENGL_PLUGIN_NAME "opengl-plugin-d"
#else
#define VULKAN_PLUGIN_NAME "vulkan-plugin"
#define OPENGL_PLUGIN_NAME "opengl-plugin"
#endif

#include <iostream>

using namespace engine;

struct Vertex {
    Vertex(const math::vec3& position, const math::vec3& normal, const math::vec2& tex_coords)
          : position(position),
            normal(normal),
            texCoords(tex_coords) {}
    math::vec3 position;
    math::vec3 normal;
    math::vec2 texCoords;
};

class LoadModelApp : public App {
public:
    LoadModelApp(const String& scene_name) : m_sceneName(scene_name), m_windowSize(800, 600) {}

protected:
    bool initialize() override {
        Main& engine = Main::GetInstance();

        m_input = InputManager::GetInstancePtr();
        m_shaderManager = ShaderManager::GetInstancePtr();
        m_render = engine.getActiveRendererPtr();
        m_window = m_render->getRenderWindowPtr().get();

        m_shaderManager->loadFromFile("model");

        SceneManager* scene_manager = SceneManager::GetInstancePtr();
        if (scene_manager) {
            scene_manager->changeActiveScene(m_sceneName);
        }

        // Mouse& mouse = m_input->GetMouse();
        // mouse.SetRelativeMouseMode(true);
        // mouse.HideCursor();

        m_camera = Camera({10, 10, 10});
        m_camera.lookAt({0, 0, 0});
        m_cameraSpeed = 2.5F;
        m_mouseSensivity = 0.1F;

        m_window->setActiveCamera(&m_camera);

        return true;
    }

    void update() override {
        m_windowSize = m_window->getSize();

        // Camera movement
        const math::vec3& camera_front = m_camera.getFrontVector();
        math::vec3 camera_forward(camera_front.x, 0, camera_front.z);
        camera_forward = math::Normalize(camera_forward);

        // Camera mouse movement
        // Mouse& mouse = m_input->GetMouse();
        // math::vec2 mouse_delta(mouse.pointer.mousedelta);
        // m_camera.Rotate(mouse_delta * m_mouse_sensivity);

        // Camera key movements
        float delta_time = getDeltaTime().asSeconds();
        float speed = m_cameraSpeed * delta_time;
        if (m_input->getButton(SDLK_w).isDown()) {
            m_camera.move(speed * camera_forward);
        }
        if (m_input->getButton(SDLK_s).isDown()) {
            m_camera.move(speed * -camera_forward);
        }
        if (m_input->getButton(SDLK_d).isDown()) {
            m_camera.move(speed * m_camera.getRightVector());
        }
        if (m_input->getButton(SDLK_a).isDown()) {
            m_camera.move(speed * -m_camera.getRightVector());
        }

        if (m_input->getButton(SDLK_SPACE).isDown()) {
            m_camera.move(speed * Camera::WORLD_UP);
        }
        if (m_input->getButton(SDLK_LSHIFT).isDown()) {
            m_camera.move(speed * -Camera::WORLD_UP);
        }

        if (m_input->getButton(SDLK_f).wentDown() || m_input->getButton(SDLK_F11).wentDown()) {
            if (m_window->isFullScreen()) {
                m_window->setFullScreen(false, false);
                m_window->resize(800, 600);
            } else {
                m_window->setFullScreen(true, true);
            }
        }

        if (m_input->getButton(SDLK_t).wentDown()) {
            Main::GetInstance().executeAsync(
                [] { LogInfo("LoadModel", "Hello Thread {}"_format(std::this_thread::get_id())); });
        }

        m_shaderManager->setActiveShader("model");
    }

    void shutdown() override {}

    String getName() override {
        return "LoadModel";
    }

    math::Vector2<int32> getWindowSize() override {
        return m_windowSize;
    }

private:
    String m_sceneName;
    math::ivec2 m_windowSize;

    Camera m_camera;
    float m_cameraSpeed;
    float m_mouseSensivity;

    InputManager* m_input;
    ShaderManager* m_shaderManager;
    Renderer* m_render;
    RenderWindow* m_window;
    std::mutex m_mutex;
};

int main(int argc, char* argv[]) {
#if PLATFORM_TYPE_IS(PLATFORM_TYPE_MOBILE)
    String renderer = "vulkan";
#else
    String renderer = (argc >= 2) ? argv[1] : "";
#endif

    String scene_name = (argc >= 3) ? argv[2] : "test2";

    String plugin;
    if (renderer == "vulkan") {
        plugin = VULKAN_PLUGIN_NAME;
    } else if (renderer == "opengl") {
        plugin = OPENGL_PLUGIN_NAME;
    } else {
        return 1;
    }

    LoadModelApp app(scene_name);

    Main engine(argc, argv);
    engine.loadPlugin(plugin);
    engine.initialize(&app);
    engine.run();
    engine.shutdown();

    return 0;
}
