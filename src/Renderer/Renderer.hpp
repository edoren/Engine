#pragma once

#include <string>

#include <Graphics/Color.hpp>
#include <Util/Precompiled.hpp>

namespace engine {

class Renderer {
public:
    Renderer();
    ~Renderer();

    bool Initialize(const String& window_title = "",
                    const math::ivec2& window_size = math::ivec2(800, 600));

    void AdvanceFrame(bool minimized);

    void ClearFrameBuffer(const Color& color);

    void ShutDown();

    void DepthTest(bool on);

    inline math::ivec2& window_size() {
        return window_size_;
    }
    inline const math::ivec2& window_size() const {
        return window_size_;
    }

    inline String& last_error() {
        return last_error_;
    }
    inline const String& last_error() const {
        return last_error_;
    }

private:
    SDL_Window* window_;
    SDL_GLContext context_;
    math::ivec2 window_size_;
    String last_error_;
};

}  // namespace engine
