#pragma once

#include <Util/Prerequisites.hpp>

namespace engine {

// This enum extends the SDL_Keycode (an int) which represent all keyboard
// keys using positive values. Negative values will represent finger / mouse
// and gamepad buttons.
// `button_map` bellow maps from one of these values to a Button.
enum {
    SDLK_POINTER1 = -10,  // Left mouse or first finger down.
    SDLK_POINTER2,        // Right mouse or second finger.
    SDLK_POINTER3,        // Middle mouse or third finger.
    SDLK_POINTER4,
    SDLK_POINTER5,
    SDLK_POINTER6,
    SDLK_POINTER7,
    SDLK_POINTER8,
    SDLK_POINTER9,
    SDLK_POINTER10,

    SDLK_PAD_UP = -20,
    SDLK_PAD_DOWN,
    SDLK_PAD_LEFT,
    SDLK_PAD_RIGHT,
    SDLK_PAD_A,
    SDLK_PAD_B
};

class InputManager;

class ENGINE_API Button {
    friend class InputManager;

public:
    Button();

    bool isDown() const;

    bool wentDown() const;

    bool wentUp() const;

private:
    void advanceFrame();

    void update(bool down);

    bool m_isDown;
    bool m_wentDown;
    bool m_wentUp;
};

}  // namespace engine
