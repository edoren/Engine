#pragma once

#include <Util/Prerequisites.hpp>

#include <Graphics/Color.hpp>
#include <System/String.hpp>

namespace engine {

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

    virtual void SwapBuffers() = 0;  // RenderTarget

    virtual void Clear(const Color& color) = 0;  // Render Target

    virtual bool IsVisible() = 0;

    void AdvanceFrame(bool minimized); // RenderTarget?

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

private:
    uint32 on_window_resize_connection;
    uint32 on_app_will_enter_background_connection;
    uint32 on_app_did_enter_background_connection;
    uint32 on_app_will_enter_foreground_connection;
    uint32 on_app_did_enter_foreground_connection;
};

}  // namespace engine
