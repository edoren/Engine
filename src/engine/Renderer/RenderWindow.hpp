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

    virtual void setFullScreen(bool fullscreen, bool isFake = false);

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
    bool m_isFullscreen;
    bool m_isVsyncEnabled;

    math::mat4 m_projection;  // RenderTarget

    const Camera* m_activeCamera;

private:
    void onWindowResizedPriv(const math::ivec2& size);

    void onAppWillEnterBackgroundPriv();
    void onAppDidEnterBackgroundPriv();
    void onAppWillEnterForegroundPriv();
    void onAppDidEnterForegroundPriv();

    SignalConnection m_onWindowResizeConnection;
    SignalConnection m_onAppWillEnterBackgroundConnection;
    SignalConnection m_onAppDidEnterBackgroundConnection;
    SignalConnection m_onAppWillEnterForegroundConnection;
    SignalConnection m_onAppDidEnterForegroundConnection;
};

}  // namespace engine
