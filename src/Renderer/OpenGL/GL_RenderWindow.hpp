#pragma once

#include <Math/Math.hpp>
#include <Renderer/RenderWindow.hpp>
#include <System/String.hpp>

#include "GL_Config.hpp"
#include "GL_Dependencies.hpp"

namespace engine {

class OPENGL_PLUGIN_API GL_RenderWindow : public RenderWindow {
public:
    GL_RenderWindow();
    ~GL_RenderWindow();

    bool Create(const String& name, const math::ivec2& size) override;

    void Destroy(void) override;

    void Reposition(int left, int top) override;

    void Resize(int width, int height) override;

    void SetFullScreen(bool fullscreen, bool is_fake) override;

    void SetVSyncEnabled(bool vsync) override;

    void SwapBuffers() override;  // RenderTarget

    void Clear(const Color& color) override;  // Render Target

    bool IsVisible() override;

    void Draw(Drawable& drawable) override;  // RenderTarget

private:
    SDL_Window* m_window;
    SDL_GLContext m_context;
};

}  // namespace engine
