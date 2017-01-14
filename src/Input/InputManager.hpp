#pragma once

#include <Util/Precompiled.hpp>

#include <Input/Button.hpp>
#include <Input/Pointer.hpp>
#include <Input/Mouse.hpp>

namespace engine {

class InputManager {
public:
    // Type definitions
    typedef std::function<void(SDL_Event*)> AppEventCallback;

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
