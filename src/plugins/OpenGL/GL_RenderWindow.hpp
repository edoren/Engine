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
    ~GL_RenderWindow() override;

    bool create(const String& name, const math::ivec2& size) override;

    void destroy(void) override;

    void resize(int width, int height) override;

    void setFullScreen(bool fullscreen, bool is_fake) override;

    void setVSyncEnabled(bool vsync) override;

    void swapBuffers() override;

    void clear(const Color& color) override;  // Render Target

protected:
    void onWindowResized(const math::ivec2& size) override;

private:
    SDL_GLContext m_context;
};

}  // namespace engine
