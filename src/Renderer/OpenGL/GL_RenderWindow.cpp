#include <Renderer/OpenGL/GL_RenderWindow.hpp>
#include <Renderer/OpenGL/GL_Utilities.hpp>

#include <iostream>  // TMP

namespace engine {

void LOG_SHIT(const String& str) {
    std::cout << str.ToUtf8() << std::endl;
}

GL_RenderWindow::GL_RenderWindow() : window_(nullptr), context_(nullptr) {}

GL_RenderWindow::~GL_RenderWindow() {
    Destroy();
}

bool GL_RenderWindow::Create(const String& name, const math::ivec2& size) {
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    math::ivec2 initial_pos(SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    Uint32 window_flags =
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;
    window_ = SDL_CreateWindow(name.GetData(), initial_pos.x, initial_pos.y,
                               size.x, size.y, window_flags);
    if (!window_) {
        // TODO: Log error
        LOG_SHIT(String("SDL_CreateWindow fail: ") + SDL_GetError());
        // TODO: Raise exception
        return false;
    }

    SDL_GetWindowSize(window_, &size_.x, &size_.y);

    context_ = SDL_GL_CreateContext(window_);
    if (!context_) {
        // TODO: Log error
        LOG_SHIT(String("SDL_GL_CreateContext fail: ") + SDL_GetError());
        // TODO: Raise exception
        return false;
    }

    GLenum status = glewInit();
    if (status != GLEW_OK) {
        // TODO: Log error
        LOG_SHIT(String("GLEW initialization failed."));
        // TODO: Raise exception
        return false;
    }

    // TODO: User enable depth test
    GL_CALL(glEnable(GL_DEPTH_TEST));

    // Update the base class attributes
    name_ = name;
    size_ = size;

    return true;
}

// void GL_RenderWindow::AdvanceFrame(bool minimized) {
//    if (minimized) {
//        // Save some cpu / battery:
//        SDL_Delay(10);
//    } else {
//        SDL_GL_SwapWindow(window_);
//    }
//    // SDL_GetWindowSize(window_, &window_size_.x, &window_size_.y);
//    // GL_CALL(glViewport(0, 0, window_size_.x, window_size_.y));
//}

void GL_RenderWindow::Destroy() {
    if (context_) {
        SDL_GL_DeleteContext(context_);
        context_ = nullptr;
    }
    if (window_) {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }
}

void GL_RenderWindow::Reposition(int left, int top) {
    if (window_) {
        // TODO check errors
        SDL_SetWindowPosition(window_, left, top);
    }
}

void GL_RenderWindow::Resize(int width, int height) {
    // TODO check errors
    if (window_ && !IsFullScreen()) {
        SDL_SetWindowSize(window_, width, height);

        // Update the base class attributes
        // TMP Update the ViewPort
        SDL_GetWindowSize(window_, &size_.x, &size_.y);
        GL_CALL(glViewport(0, 0, size_.x, size_.y));
    }
}

void GL_RenderWindow::SetFullScreen(bool fullscreen, bool is_fake) {
    // TODO check errors
    if (window_) {
        is_fullscreen_ = fullscreen;
        Uint32 flag = 0;
        if (fullscreen) {
            flag = (is_fake) ? SDL_WINDOW_FULLSCREEN_DESKTOP
                             : SDL_WINDOW_FULLSCREEN;
        }
        SDL_SetWindowFullscreen(window_, flag);

        // Update the base class attributes
        // TMP Update the ViewPort
        SDL_GetWindowSize(window_, &size_.x, &size_.y);
        GL_CALL(glViewport(0, 0, size_.x, size_.y));
    }
}

void GL_RenderWindow::SetVSyncEnabled(bool vsync) {
    if (SDL_GL_SetSwapInterval(vsync ? 1 : 0) == 0) {
        is_vsync_enable_ = vsync;
    } else {
        is_vsync_enable_ = false;
    }
}

void GL_RenderWindow::SwapBuffers() {
    SDL_GL_SwapWindow(window_);
    // RenderWindow::SwapBuffers();
}

void GL_RenderWindow::Clear(const Color& color) {  // RenderTarget
    GL_CALL(glClearColor(color.r, color.g, color.b, color.a));
    GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}

bool GL_RenderWindow::IsVisible() {
    Uint32 flags = SDL_WINDOW_HIDDEN | SDL_WINDOW_MINIMIZED;
    Uint32 mask = SDL_GetWindowFlags(window_);
    return (mask & flags) == 0;
}

}  // namespace engine
