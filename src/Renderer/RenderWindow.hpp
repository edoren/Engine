#pragma once

#include <Util/Prerequisites.hpp>

#include <Graphics/Color.hpp>
#include <Renderer/RenderStates.hpp>
#include <System/SignalConnection.hpp>
#include <System/String.hpp>

namespace engine {

class Camera;
class Drawable;
class Mesh;

class ENGINE_API RenderWindow {
public:
    RenderWindow();

    virtual ~RenderWindow();

    virtual bool Create(const String& name, const math::ivec2& size) = 0;

    virtual void Destroy() = 0;

    virtual void Reposition(int left, int top) = 0;

    virtual void Resize(int width, int height) = 0;

    virtual void SetFullScreen(bool fullscreen, bool is_fake = false) = 0;

    virtual void SetVSyncEnabled(bool vsync) = 0;

    // RenderTarget
    virtual void SwapBuffers() = 0;

    // RenderTarget
    virtual void Clear(const Color& color) = 0;

    virtual bool IsVisible() = 0;

    // RenderTarget
    virtual void Draw(const Drawable& drawable);
    virtual void Draw(const Mesh& mesh, const RenderStates& states);

    // RenderTarget
    void SetActiveCamera(const Camera* camera);
    const Camera* GetActiveCamera() const;

    // RenderTarget?
    void AdvanceFrame(bool minimized);

    const String& GetName() const;

    const math::ivec2& GetSize() const;

    bool IsVSyncEnabled() const;

    bool IsFullScreen() const;

    const math::mat4& GetProjectionMatrix() const;

protected:
    // RenderTarget
    virtual void UpdateProjectionMatrix();

    virtual void OnWindowResized(const math::ivec2& size);

    virtual void OnAppWillEnterBackground();
    virtual void OnAppDidEnterBackground();
    virtual void OnAppWillEnterForeground();
    virtual void OnAppDidEnterForeground();

protected:
    String m_name;
    math::ivec2 m_size;  // If updated UpdateProjectionMatrix should be called
    bool m_is_fullscreen;
    bool m_is_vsync_enable;

    math::mat4 m_projection;  // RenderTarget

    const Camera* m_active_camera;

private:
    SignalConnection on_window_resize_connection;
    SignalConnection on_app_will_enter_background_connection;
    SignalConnection on_app_did_enter_background_connection;
    SignalConnection on_app_will_enter_foreground_connection;
    SignalConnection on_app_did_enter_foreground_connection;
};

}  // namespace engine
