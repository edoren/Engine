#include <Renderer/RenderWindow.hpp>

#include <Input/InputManager.hpp>

namespace engine {

RenderWindow::RenderWindow() {
    auto& input = InputManager::GetInstance();

    on_window_resize_connection =
        input.OnWindowResized.Connect(this, &RenderWindow::OnWindowResized);

    on_app_will_enter_background_connection =
        input.OnAppWillEnterBackground.Connect(
            this, &RenderWindow::OnAppWillEnterBackground);
    on_app_did_enter_background_connection =
        input.OnAppDidEnterBackground.Connect(
            this, &RenderWindow::OnAppDidEnterBackground);
    on_app_will_enter_foreground_connection =
        input.OnAppWillEnterForeground.Connect(
            this, &RenderWindow::OnAppWillEnterForeground);
    on_app_did_enter_foreground_connection =
        input.OnAppDidEnterForeground.Connect(
            this, &RenderWindow::OnAppDidEnterForeground);
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

void RenderWindow::OnWindowResized(const math::ivec2& size) {
    ENGINE_UNUSED(size);
}

void RenderWindow::OnAppWillEnterBackground() {}
void RenderWindow::OnAppDidEnterBackground() {}
void RenderWindow::OnAppWillEnterForeground() {}
void RenderWindow::OnAppDidEnterForeground() {}

}  // namespace engine
