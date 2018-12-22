#include <Input/InputManager.hpp>
#include <Renderer/Drawable.hpp>
#include <Renderer/Mesh.hpp>
#include <Renderer/RenderWindow.hpp>

namespace engine {

RenderWindow::RenderWindow()
      : m_name(),
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
        input.OnWindowResized.Connect(*this, &RenderWindow::OnWindowResized);

    on_app_will_enter_background_connection =
        input.OnAppWillEnterBackground.Connect(
            *this, &RenderWindow::OnAppWillEnterBackground);
    on_app_did_enter_background_connection =
        input.OnAppDidEnterBackground.Connect(
            *this, &RenderWindow::OnAppDidEnterBackground);
    on_app_will_enter_foreground_connection =
        input.OnAppWillEnterForeground.Connect(
            *this, &RenderWindow::OnAppWillEnterForeground);
    on_app_did_enter_foreground_connection =
        input.OnAppDidEnterForeground.Connect(
            *this, &RenderWindow::OnAppDidEnterForeground);
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

    // Update the base class attributes
    m_name = name;
    OnWindowResized(m_size);  // This update m_size and the projection matrix

    return true;
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

void RenderWindow::OnWindowResized(const math::ivec2& size) {
    m_size = size;
    UpdateProjectionMatrix();
}

void RenderWindow::OnAppWillEnterBackground() {}
void RenderWindow::OnAppDidEnterBackground() {}
void RenderWindow::OnAppWillEnterForeground() {}
void RenderWindow::OnAppDidEnterForeground() {}

}  // namespace engine
