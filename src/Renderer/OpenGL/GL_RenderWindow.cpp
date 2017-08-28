#include "GL_RenderWindow.hpp"
#include "GL_Utilities.hpp"

#include <System/LogManager.hpp>

namespace engine {

namespace {

const String sTag("GL_RenderWindow");

}  // namespace

GL_RenderWindow::GL_RenderWindow() : m_window(nullptr), m_context(nullptr) {}

GL_RenderWindow::~GL_RenderWindow() {
    Destroy();
}

bool GL_RenderWindow::Create(const String& name, const math::ivec2& size) {
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_CORE);

    math::ivec2 initial_pos(SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    Uint32 window_flags =
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;
    m_window = SDL_CreateWindow(name.GetData(), initial_pos.x, initial_pos.y,
                                size.x, size.y, window_flags);
    if (!m_window) {
        LogError(sTag, SDL_GetError());
        return false;
    }

    SDL_GetWindowSize(m_window, &m_size.x, &m_size.y);

    m_context = SDL_GL_CreateContext(m_window);
    if (!m_context) {
        LogError(sTag, SDL_GetError());
        return false;
    }

    String opengl_vendor =
        reinterpret_cast<const char8*>(glGetString(GL_VENDOR));
    String opengl_renderer =
        reinterpret_cast<const char8*>(glGetString(GL_RENDERER));
    String opengl_version =
        reinterpret_cast<const char8*>(glGetString(GL_VERSION));
    String glsl_version = reinterpret_cast<const char8*>(
        glGetString(GL_SHADING_LANGUAGE_VERSION));

    LogInfo(sTag, "OpenGL Vendor: " + opengl_vendor);
    LogInfo(sTag, "OpenGL Renderer: " + opengl_renderer);
    LogInfo(sTag, "OpenGL Version: " + opengl_version);
    LogInfo(sTag, "GLSL Version: " + glsl_version);

#if PLATFORM_TYPE_IS(PLATFORM_TYPE_DESKTOP)
    GLenum status = glewInit();
    if (status != GLEW_OK) {
        LogError(sTag, "GLEW initialization failed.");
        return false;
    }
#endif

    // TODO: User enable depth test
    GL_CALL(glEnable(GL_DEPTH_TEST));

    // Update the base class attributes
    m_name = name;
    m_size = size;

    return true;
}

// void GL_RenderWindow::AdvanceFrame(bool minimized) {
//    if (minimized) {
//        // Save some cpu / battery:
//        SDL_Delay(10);
//    } else {
//        SDL_GL_SwapWindow(m_window);
//    }
//    // SDL_GetWindowSize(m_window, &m_window_size.x, &m_window_size.y);
//    // GL_CALL(glViewport(0, 0, m_window_size.x, m_window_size.y));
//}

void GL_RenderWindow::Destroy() {
    if (m_context) {
        SDL_GL_DeleteContext(m_context);
        m_context = nullptr;
    }
    if (m_window) {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }
}

void GL_RenderWindow::Reposition(int left, int top) {
    if (m_window) {
        // TODO check errors
        SDL_SetWindowPosition(m_window, left, top);
    }
}

void GL_RenderWindow::Resize(int width, int height) {
    // TODO check errors
    if (m_window && !IsFullScreen()) {
        SDL_SetWindowSize(m_window, width, height);

        // Update the base class attributes
        // TMP Update the ViewPort
        SDL_GetWindowSize(m_window, &m_size.x, &m_size.y);
        GL_CALL(glViewport(0, 0, m_size.x, m_size.y));
    }
}

void GL_RenderWindow::SetFullScreen(bool fullscreen, bool is_fake) {
    // TODO check errors
    if (m_window) {
        m_is_fullscreen = fullscreen;
        Uint32 flag = 0;
        if (fullscreen) {
            flag = (is_fake) ? SDL_WINDOW_FULLSCREEN_DESKTOP
                             : SDL_WINDOW_FULLSCREEN;
        }
        SDL_SetWindowFullscreen(m_window, flag);

        // Update the base class attributes
        // TMP Update the ViewPort
        SDL_GetWindowSize(m_window, &m_size.x, &m_size.y);
        GL_CALL(glViewport(0, 0, m_size.x, m_size.y));
    }
}

void GL_RenderWindow::SetVSyncEnabled(bool vsync) {
    if (SDL_GL_SetSwapInterval(vsync ? 1 : 0) == 0) {
        m_is_vsync_enable = vsync;
    } else {
        m_is_vsync_enable = false;
    }
}

void GL_RenderWindow::SwapBuffers() {
    SDL_GL_SwapWindow(m_window);
    // RenderWindow::SwapBuffers();
}

void GL_RenderWindow::Clear(const Color& color) {  // RenderTarget
    GL_CALL(glClearColor(color.r, color.g, color.b, color.a));
    GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}

bool GL_RenderWindow::IsVisible() {
    Uint32 flags = SDL_WINDOW_HIDDEN | SDL_WINDOW_MINIMIZED;
    Uint32 mask = SDL_GetWindowFlags(m_window);
    return (mask & flags) == 0;
}

}  // namespace engine
