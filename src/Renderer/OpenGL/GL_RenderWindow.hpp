#pragma once

#include <Util/Prerequisites.hpp>

#include <Math/Math.hpp>
#include <Renderer/RenderWindow.hpp>
#include <System/String.hpp>

#include <Renderer/OpenGL/GL_Prerequisites.hpp>

namespace engine {

class ENGINE_API GL_RenderWindow : public RenderWindow {
public:
    GL_RenderWindow();
    ~GL_RenderWindow();

    ////////////////////////////////////////////////////////////////
    // Overridden
    virtual bool Create(const String& name, const math::ivec2& size);

    virtual void Destroy(void);

    virtual void Reposition(int left, int top);

    virtual void Resize(int width, int height);

    virtual void SetFullScreen(bool fullscreen, bool is_fake);

    virtual void SetVSyncEnabled(bool vsync);

    virtual void SwapBuffers();  // RenderTarget

    virtual void Clear(const Color& color);  // Render Target

    virtual bool IsVisible();
    ////////////////////////////////////////////////////////////////

private:
    SDL_Window* window_;
    SDL_GLContext context_;
};

}  // namespace engine
