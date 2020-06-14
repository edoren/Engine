#pragma once

#include <Util/Prerequisites.hpp>

#include <Graphics/Color.hpp>
#include <System/SignalConnection.hpp>
#include <System/String.hpp>

namespace engine {

class Camera;
class Mesh;

class ENGINE_API RenderWindow {
public:
    RenderWindow();

    virtual ~RenderWindow();

    virtual bool create(const String& name, const math::ivec2& size);

    virtual void destroy();

    void reposition(int left, int top);

    virtual void resize(int width, int height);

    virtual void setFullScreen(bool fullscreen, bool is_fake = false);

    virtual void setVSyncEnabled(bool vsync) = 0;

    // RenderTarget
    virtual void swapBuffers() = 0;

    // RenderTarget
    virtual void clear(const Color& color) = 0;

    bool isVisible();

    // RenderTarget
    void setActiveCamera(const Camera* camera);
    const Camera* getActiveCamera() const;

    // RenderTarget?
    void advanceFrame(bool minimized);

    const String& getName() const;

    const math::ivec2& getSize() const;

    bool isVSyncEnabled() const;

    bool isFullScreen() const;

    const math::mat4& getProjectionMatrix() const;

protected:
    // RenderTarget
    virtual void updateProjectionMatrix();

    virtual void onWindowResized(const math::ivec2& size);

    virtual void onAppWillEnterBackground();
    virtual void onAppDidEnterBackground();
    virtual void onAppWillEnterForeground();
    virtual void onAppDidEnterForeground();

    void* m_window;

    String m_name;
    math::ivec2 m_size;  // If updated UpdateProjectionMatrix should be called
    bool m_is_fullscreen;
    bool m_is_vsync_enable;

    math::mat4 m_projection;  // RenderTarget

    const Camera* m_active_camera;

private:
    void onWindowResizedPriv(const math::ivec2& size);

    void onAppWillEnterBackgroundPriv();
    void onAppDidEnterBackgroundPriv();
    void onAppWillEnterForegroundPriv();
    void onAppDidEnterForegroundPriv();

    SignalConnection on_window_resize_connection;
    SignalConnection on_app_will_enter_background_connection;
    SignalConnection on_app_did_enter_background_connection;
    SignalConnection on_app_will_enter_foreground_connection;
    SignalConnection on_app_did_enter_foreground_connection;
};

}  // namespace engine
