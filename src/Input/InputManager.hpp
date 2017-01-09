#pragma once

#include <Util/Precompiled.hpp>

namespace engine {

// This enum extends the SDL_Keycode (an int) which represent all keyboard
// keys using positive values. Negative values will represent finger / mouse
// and gamepad buttons.
// `button_map` below maps from one of these values to a Button.
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

class Button {
public:
    Button() : is_down_(false) {
        AdvanceFrame();
    }

    void AdvanceFrame() {
        went_down_ = went_up_ = false;
    }

    void Update(bool down);

    inline bool IsDown() const {
        return is_down_;
    }

    inline bool WentDown() const {
        return went_down_;
    }

    inline bool WentUp() const {
        return went_up_;
    }

private:
    bool is_down_;
    bool went_down_;
    bool went_up_;
};

struct Pointer {
    SDL_FingerID id;
    math::ivec2 mousepos;
    math::ivec2 mousedelta;
    bool used;

    Pointer() : id(0), mousepos(-1), mousedelta(0), used(false){};
};

class Mouse : NonCopyable {
    friend class InputManager;

private:
    Mouse(Pointer& pointer) : pointer(pointer){};

public:
    void SetRelativeMouseMode(bool enable) {
        if (enable) {
            SDL_SetRelativeMouseMode(SDL_TRUE);
        } else {
            SDL_SetRelativeMouseMode(SDL_FALSE);
        }
    }

    void ShowCursor() {
        SDL_ShowCursor(1);
    }
    void HideCursor() {
        SDL_ShowCursor(0);
    }

    Pointer& pointer;
};

class InputManager {
public:
    // All current touch screens.
    static const int kMaxSimultanuousPointers = 10;

    InputManager();

    void Initialize();

    void AdvanceFrame(math::ivec2* window_size);

    Button& GetButton(int button);

    Button& GetPointerButton(SDL_FingerID pointer);

    Mouse& GetMouse() {
        static std::unique_ptr<Mouse> instance(new Mouse(pointers_[0]));
        return *instance;
    }

    typedef std::function<void(SDL_Event*)> AppEventCallback;
    void AddEventCallback(AppEventCallback callback);

    inline bool minimized() {
        return minimized_;
    }

    inline bool exit_requested() {
        return exit_requested_;
    }

private:
    static int HandleAppEvents(void* userdata, SDL_Event* event);

private:
    bool minimized_;
    bool exit_requested_;
    std::vector<AppEventCallback> app_event_callbacks_;
    std::vector<Pointer> pointers_;
    std::map<int, Button> button_map_;
    math::ivec2 mousewheel_delta_;
};

}  // namespace engine
