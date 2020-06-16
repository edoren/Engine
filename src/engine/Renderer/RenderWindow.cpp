#include <Input/InputManager.hpp>
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
        m_isFullscreen(false),
        m_isVsyncEnabled(false),
        m_activeCamera(nullptr) {
    auto& input = InputManager::GetInstance();

    m_onWindowResizeConnection = input.onWindowResized.connect(*this, &RenderWindow::onWindowResizedPriv);

    m_onAppWillEnterBackgroundConnection =
        input.onAppWillEnterBackground.connect(*this, &RenderWindow::onAppWillEnterBackgroundPriv);
    m_onAppDidEnterBackgroundConnection =
        input.onAppDidEnterBackground.connect(*this, &RenderWindow::onAppDidEnterBackgroundPriv);
    m_onAppWillEnterForegroundConnection =
        input.onAppWillEnterForeground.connect(*this, &RenderWindow::onAppWillEnterForegroundPriv);
    m_onAppDidEnterForegroundConnection =
        input.onAppDidEnterForeground.connect(*this, &RenderWindow::onAppDidEnterForegroundPriv);
}

RenderWindow::~RenderWindow() {
    auto& input = InputManager::GetInstance();

    input.onWindowResized.disconnect(m_onWindowResizeConnection);

    input.onAppWillEnterBackground.disconnect(m_onAppWillEnterBackgroundConnection);
    input.onAppDidEnterBackground.disconnect(m_onAppDidEnterBackgroundConnection);
    input.onAppWillEnterForeground.disconnect(m_onAppWillEnterForegroundConnection);
    input.onAppDidEnterForeground.disconnect(m_onAppDidEnterForegroundConnection);
}

bool RenderWindow::create(const String& name, const math::ivec2& size) {
    ENGINE_UNUSED(size);

    m_name = name;

    math::ivec2 actual_size;
    if (m_window) {
        SDL_GetWindowSize(reinterpret_cast<SDL_Window*>(m_window), &actual_size.x, &actual_size.y);
        m_size = actual_size;
    } else {
        LogDebug(sTag, "m_window should be created before calling RenderWindow::Create");
        return false;
    }

    LogInfo(sTag, "Created Window '{}' with size: [{}, {}]"_format(name, m_size.x, m_size.y));

    // Update the base class attributes
    updateProjectionMatrix();

    return true;
}

void RenderWindow::destroy() {
    if (auto* window = reinterpret_cast<SDL_Window*>(m_window)) {
        SDL_DestroyWindow(window);
        m_window = nullptr;
    }
}

void RenderWindow::reposition(int left, int top) {
    if (auto* window = reinterpret_cast<SDL_Window*>(m_window)) {
        SDL_SetWindowPosition(window, left, top);
    }
}

void RenderWindow::resize(int width, int height) {
    // TODO check errors
    auto* window = reinterpret_cast<SDL_Window*>(m_window);
    if (window && !isFullScreen()) {
        SDL_SetWindowSize(window, width, height);

        // Update the base class attributes
        onWindowResizedPriv(m_size);
    }
}

void RenderWindow::setFullScreen(bool fullscreen, bool is_fake) {
    // TODO: check errors
    if (auto* window = reinterpret_cast<SDL_Window*>(m_window)) {
        Uint32 flag = 0;
        if (fullscreen) {
            flag = (is_fake) ? SDL_WINDOW_FULLSCREEN_DESKTOP : SDL_WINDOW_FULLSCREEN;
        }
        SDL_SetWindowFullscreen(window, flag);

        // Update the base class attributes
        onWindowResizedPriv(m_size);
        SDL_SetWindowSize(window, m_size.x, m_size.y);
        m_isFullscreen = fullscreen;
    }
}

void RenderWindow::setActiveCamera(const Camera* camera) {
    m_activeCamera = camera;
}

const Camera* RenderWindow::getActiveCamera() const {
    return m_activeCamera;
}

void RenderWindow::advanceFrame(bool minimized) {
    ENGINE_UNUSED(minimized);
}

const String& RenderWindow::getName() const {
    return m_name;
};

const math::ivec2& RenderWindow::getSize() const {
    return m_size;
};

bool RenderWindow::isVSyncEnabled() const {
    return m_isVsyncEnabled;
};

bool RenderWindow::isFullScreen() const {
    return m_isFullscreen;
}

const math::mat4& RenderWindow::getProjectionMatrix() const {
    return m_projection;
}

void RenderWindow::updateProjectionMatrix() {
    float fov = math::Radians(45.F);
    float aspect_ratio = m_size.x / static_cast<float>(m_size.y);
    float z_near = 0.1F;
    float z_far = 100.0F;
    m_projection = math::Perspective(fov, aspect_ratio, z_near, z_far);
}

void RenderWindow::onWindowResized(const math::ivec2& /*unused*/) {}

void RenderWindow::onAppWillEnterBackground() {
    LogInfo(sTag, "OnAppWillEnterBackground");
}

void RenderWindow::onAppDidEnterBackground() {
    LogInfo(sTag, "OnAppDidEnterBackground");
}

void RenderWindow::onAppWillEnterForeground() {
    LogInfo(sTag, "OnAppWillEnterForeground");
}

void RenderWindow::onAppDidEnterForeground() {
    LogInfo(sTag, "OnAppDidEnterForeground");
}

bool RenderWindow::isVisible() {
    Uint32 flags = SDL_WINDOW_HIDDEN | SDL_WINDOW_MINIMIZED;
    Uint32 mask = SDL_GetWindowFlags(reinterpret_cast<SDL_Window*>(m_window));
    return (mask & flags) == 0;
}

void RenderWindow::onWindowResizedPriv(const math::ivec2& size) {
    ENGINE_UNUSED(size);

    if (isFullScreen()) {
        return;
    }

    // Get the new window size from the active window
    math::ivec2 new_size;
    SDL_GetWindowSize(reinterpret_cast<SDL_Window*>(m_window), &new_size.x, &new_size.y);
    if (new_size != m_size) {
        m_size = new_size;
        LogDebug(sTag, "OnWindowResized {}x{}"_format(m_size.x, m_size.y));
        updateProjectionMatrix();
        onWindowResized(m_size);
    }
}

void RenderWindow::onAppWillEnterBackgroundPriv() {
    LogDebug(sTag, "OnAppWillEnterBackground");
    onAppWillEnterBackground();
}

void RenderWindow::onAppDidEnterBackgroundPriv() {
    LogDebug(sTag, "OnAppDidEnterBackground");
    onAppDidEnterBackground();
}

void RenderWindow::onAppWillEnterForegroundPriv() {
    LogDebug(sTag, "OnAppWillEnterForeground");
    onAppWillEnterForeground();
}

void RenderWindow::onAppDidEnterForegroundPriv() {
    LogDebug(sTag, "OnAppDidEnterForeground");
    onAppDidEnterForeground();
}

}  // namespace engine
