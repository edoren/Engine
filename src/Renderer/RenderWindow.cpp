#include <Renderer/RenderWindow.hpp>

namespace engine {

RenderWindow::RenderWindow() {}

RenderWindow::~RenderWindow() {}

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
