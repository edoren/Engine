#include "Core/Main.hpp"

#include <Renderer/OpenGL/GL_Renderer.hpp>

namespace engine {

template <>
Main* Singleton<Main>::s_instance = nullptr;

Main& Main::GetInstance() {
    assert(s_instance);
    return (*s_instance);
}

Main* Main::GetInstancePtr() {
    return s_instance;
}

Main::Main(int argc, char* argv[]) : is_initialized_(false) {
    filesystem::Path resource_dir = filesystem::Absolute(argv[0]).ParentPath();

    m_input = new InputManager();
    m_resources = new ResourceManager(resource_dir);
}

Main::~Main() {
    ShutDown();
    delete m_input;
    delete m_resources;
}

bool Main::Initialize() {
    if (!is_initialized_) {
        SDL_Init(0);

        // SDL_LogSetAllPriority(SDL_LOG_PRIORITY_INFO);  // TMP

        m_renderer = new GL_Renderer();

        m_input->Initialize();
        m_resources->Initialize();
        m_renderer->Initialize();

        is_initialized_ = true;
    }
    return is_initialized_;
}

void Main::ShutDown() {
    if (is_initialized_) {
        m_input->ShutDown();
        m_resources->ShutDown();
        m_renderer->ShutDown();

        delete m_renderer;
        m_renderer = nullptr;

        SDL_Quit();

        is_initialized_ = false;
    }
}

Renderer& Main::GetRenderer() {
    assert(m_renderer);
    return *m_renderer;
}

Renderer* Main::GetRendererPtr() {
    return m_renderer;
}

}  // namespace engine
