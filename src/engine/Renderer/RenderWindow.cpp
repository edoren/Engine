#include <Input/InputManager.hpp>
#include <Renderer/Drawable.hpp>
#include <Renderer/Mesh.hpp>
#include <Renderer/RenderWindow.hpp>
#include <System/LogManager.hpp>
#include <System/StringFormat.hpp>

#include <SDL2.h>

namespace engine {

namespace {

const String sTag("RenderWindow");

}  // namespace

RenderWindow::RenderWindow()
      : m_window(nullptr),
        m_name(),
        m_size(),
        m_is_fullscreen(false),
        m_is_vsync_enable(false),
        m_active_camera(nullptr),
        on_window_resize_connection(),
        on_app_will_enter_background_connection(),
        on_app_did_enter_background_connection(),
        on_app_will_enter_foreground_connection(),
        on_app_did_enter_foreground_connection() {
    auto& input = InputManager::GetInstance();

    on_window_resize_connection =
        input.OnWindowResized.Connect(*this, &RenderWindow::_OnWindowResized);

    on_app_will_enter_background_connection =
        input.OnAppWillEnterBackground.Connect(
            *this, &RenderWindow::_OnAppWillEnterBackground);
    on_app_did_enter_background_connection =
        input.OnAppDidEnterBackground.Connect(
            *this, &RenderWindow::_OnAppDidEnterBackground);
    on_app_will_enter_foreground_connection =
        input.OnAppWillEnterForeground.Connect(
            *this, &RenderWindow::_OnAppWillEnterForeground);
    on_app_did_enter_foreground_connection =
        input.OnAppDidEnterForeground.Connect(
            *this, &RenderWindow::_OnAppDidEnterForeground);
}

RenderWindow::~RenderWindow() {
    auto& input = InputManager::GetInstance();

    input.OnWindowResized.Disconnect(on_window_resize_connection);

    input.OnAppWillEnterBackground.Disconnect(
        on_app_will_enter_background_connection);
    input.OnAppDidEnterBackground.Disconnect(
        on_app_did_enter_background_connection);
    input.OnAppWillEnterForeground.Disconnect(
        on_app_will_enter_foreground_connection);
    input.OnAppDidEnterForeground.Disconnect(
        on_app_did_enter_foreground_connection);
}

bool RenderWindow::Create(const String& name, const math::ivec2& size) {
    ENGINE_UNUSED(size);

    LogInfo(sTag, "Creating Window '{}' with size: {}x{}"_format(name, size.x,
                                                                 size.y));

    // Update the base class attributes
    m_name = name;
    m_size = size;
    UpdateProjectionMatrix();

    return true;
}

void RenderWindow::Destroy() {
    if (SDL_Window* window = reinterpret_cast<SDL_Window*>(m_window)) {
        SDL_DestroyWindow(window);
        m_window = nullptr;
    }
}

void RenderWindow::Reposition(int left, int top) {
    if (SDL_Window* window = reinterpret_cast<SDL_Window*>(m_window)) {
        SDL_SetWindowPosition(window, left, top);
    }
}

void RenderWindow::Resize(int width, int height) {
    // TODO check errors
    SDL_Window* window = reinterpret_cast<SDL_Window*>(m_window);
    if (window && !IsFullScreen()) {
        SDL_SetWindowSize(window, width, height);

        // Update the base class attributes
        _OnWindowResized(m_size);
    }
}

void RenderWindow::SetFullScreen(bool fullscreen, bool is_fake) {
    // TODO: check errors
    if (SDL_Window* window = reinterpret_cast<SDL_Window*>(m_window)) {
        Uint32 flag = 0;
        if (fullscreen) {
            flag = (is_fake) ? SDL_WINDOW_FULLSCREEN_DESKTOP
                             : SDL_WINDOW_FULLSCREEN;
        }
        SDL_SetWindowFullscreen(window, flag);

        // Update the base class attributes
        _OnWindowResized(m_size);
        SDL_SetWindowSize(window, m_size.x, m_size.y);
        m_is_fullscreen = fullscreen;
    }
}

void RenderWindow::Draw(const Mesh& mesh, const RenderStates& states) {
    mesh.Draw(*this, states);
}

void RenderWindow::SetActiveCamera(const Camera* camera) {
    m_active_camera = camera;
}

const Camera* RenderWindow::GetActiveCamera() const {
    return m_active_camera;
}

void RenderWindow::AdvanceFrame(bool minimized) {
    ENGINE_UNUSED(minimized);
}

const String& RenderWindow::GetName() const {
    return m_name;
};

const math::ivec2& RenderWindow::GetSize() const {
    return m_size;
};

bool RenderWindow::IsVSyncEnabled() const {
    return m_is_vsync_enable;
};

bool RenderWindow::IsFullScreen() const {
    return m_is_fullscreen;
}

const math::mat4& RenderWindow::GetProjectionMatrix() const {
    return m_projection;
}

void RenderWindow::UpdateProjectionMatrix() {
    float fov = math::Radians(45.f);
    float aspect_ratio = m_size.x / static_cast<float>(m_size.y);
    float z_near = 0.1f;
    float z_far = 100.0f;
    m_projection = math::Perspective(fov, aspect_ratio, z_near, z_far);
}

void RenderWindow::OnWindowResized(const math::ivec2&) {}

void RenderWindow::OnAppWillEnterBackground() {}

void RenderWindow::OnAppDidEnterBackground() {}

void RenderWindow::OnAppWillEnterForeground() {}

void RenderWindow::OnAppDidEnterForeground() {}

bool RenderWindow::IsVisible() {
    Uint32 flags = SDL_WINDOW_HIDDEN | SDL_WINDOW_MINIMIZED;
    Uint32 mask = SDL_GetWindowFlags(reinterpret_cast<SDL_Window*>(m_window));
    return (mask & flags) == 0;
}

void RenderWindow::_OnWindowResized(const math::ivec2& size) {
    ENGINE_UNUSED(size);

    if (IsFullScreen()) {
        return;
    }

    // Get the new window size from the active window
    math::ivec2 new_size;
    SDL_GetWindowSize(reinterpret_cast<SDL_Window*>(m_window), &new_size.x,
                      &new_size.y);
    if (new_size != m_size) {
        m_size = new_size;
        LogDebug(sTag, "OnWindowResized {}x{}"_format(m_size.x, m_size.y));
        UpdateProjectionMatrix();
        OnWindowResized(m_size);
    }
}

void RenderWindow::_OnAppWillEnterBackground() {
    LogDebug(sTag, "OnAppWillEnterBackground");
    OnAppWillEnterBackground();
}

void RenderWindow::_OnAppDidEnterBackground() {
    LogDebug(sTag, "OnAppDidEnterBackground");
    OnAppDidEnterBackground();
}

void RenderWindow::_OnAppWillEnterForeground() {
    LogDebug(sTag, "OnAppWillEnterForeground");
    OnAppWillEnterForeground();
}

void RenderWindow::_OnAppDidEnterForeground() {
    LogDebug(sTag, "OnAppDidEnterForeground");
    OnAppDidEnterForeground();
}

}  // namespace engine