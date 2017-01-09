#include <Renderer/GLUtilities.hpp>
#include <Renderer/Renderer.hpp>

namespace engine {

Renderer::Renderer() : window_(nullptr), window_size_(math::ivec2(0, 0)) {}

Renderer::~Renderer() {
    ShutDown();
}

bool Renderer::Initialize(const String& window_title,
                          const math::ivec2& window_size) {
    if (SDL_Init(SDL_INIT_VIDEO)) {
        last_error_ = String("SDL_Init fail: ") + SDL_GetError();
        return false;
    }

    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_INFO);

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    window_ = SDL_CreateWindow(
        window_title.GetData(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        window_size.x, window_size.y,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (!window_) {
        last_error_ = String("SDL_CreateWindow fail: ") + SDL_GetError();
        return false;
    }

    SDL_GetWindowSize(window_, &window_size_.x, &window_size_.y);

    context_ = SDL_GL_CreateContext(window_);
    if (!context_) {
        last_error_ =
            String("SDL_GL_CreateContext fail: ") + SDL_GetError();
        return false;
    }

    GLenum status = glewInit();
    if (status != GLEW_OK) {
        last_error_ = String("glewInit fail.");
        return false;
    }

    return true;
}

void Renderer::AdvanceFrame(bool minimized) {
    if (minimized) {
        // Save some cpu / battery:
        SDL_Delay(10);
    } else {
        SDL_GL_SwapWindow(window_);
    }
    SDL_GetWindowSize(window_, &window_size_.x, &window_size_.y);
    GL_CALL(glViewport(0, 0, window_size_.x, window_size_.y));
    DepthTest(true);
}

void Renderer::ClearFrameBuffer(const Color& color) {
    GL_CALL(glClearColor(color.r, color.g, color.b, color.a));
    GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}

void Renderer::ShutDown() {
    if (context_) {
        SDL_GL_DeleteContext(context_);
        context_ = nullptr;
    }
    if (window_) {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }
}

void Renderer::DepthTest(bool on) {
    if (on) {
        GL_CALL(glEnable(GL_DEPTH_TEST));
    } else {
        GL_CALL(glDisable(GL_DEPTH_TEST));
    }
}

}  // namespace engine
