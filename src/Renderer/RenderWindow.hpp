#pragma once

#include <Util/Prerequisites.hpp>

#include <Graphics/Color.hpp>
#include <System/SignalConnection.hpp>
#include <System/String.hpp>

namespace engine {

class Camera;
class Drawable;
struct UniformBufferObject;

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
    virtual void Draw(Drawable& drawable);

    // RenderTarget?
    virtual void SetUniformBufferObject(const UniformBufferObject& ubo);

    // RenderTarget
    void SetActiveCamera(const Camera* camera);

    // RenderTarget?
    void AdvanceFrame(bool minimized);

    const String& GetName() const;

    const math::ivec2& GetSize() const;

    bool IsVSyncEnabled() const;

    bool IsFullScreen() const;

private:
    virtual void OnWindowResized(const math::ivec2& size);

    virtual void OnAppWillEnterBackground();
    virtual void OnAppDidEnterBackground();
    virtual void OnAppWillEnterForeground();
    virtual void OnAppDidEnterForeground();

protected:
    String m_name;
    math::ivec2 m_size;
    bool m_is_fullscreen;
    bool m_is_vsync_enable;

    const Camera* m_active_camera;

private:
    SignalConnection on_window_resize_connection;
    SignalConnection on_app_will_enter_background_connection;
    SignalConnection on_app_did_enter_background_connection;
    SignalConnection on_app_will_enter_foreground_connection;
    SignalConnection on_app_did_enter_foreground_connection;
};

}  // namespace engine
