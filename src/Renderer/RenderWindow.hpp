#pragma once

#include <Util/Prerequisites.hpp>

#include <Graphics/Color.hpp>
#include <System/String.hpp>

namespace engine {

class ENGINE_API RenderWindow {
public:
    virtual ~RenderWindow() {}

    virtual bool Create(const String& name, const math::ivec2& size) = 0;

    virtual void Destroy() = 0;

    virtual void Reposition(int left, int top) = 0;

    virtual void Resize(int width, int height) = 0;

    virtual void SetFullScreen(bool fullscreen, bool is_fake = false) = 0;

    virtual void SetVSyncEnabled(bool vsync) = 0;

    virtual void SwapBuffers() = 0;  // RenderTarget

    virtual void Clear(const Color& color) = 0;  // Render Target

    virtual bool IsVisible() = 0;

    void AdvanceFrame(bool /*minimized*/) {}  // RenderTarget?

    const String& GetName() const {
        return name_;
    };

    const math::ivec2& GetSize() const {
        return size_;
    };

    bool IsVSyncEnabled() const {
        return is_vsync_enable_;
    };

    bool IsFullScreen() const {
        return is_fullscreen_;
    }

protected:
    String name_;
    math::ivec2 size_;
    bool is_fullscreen_;
    bool is_vsync_enable_;
};

}  // namespace engine
