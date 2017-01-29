#pragma once

#include <Util/Precompiled.hpp>

#include <Input/Button.hpp>
#include <Input/Mouse.hpp>
#include <Input/Pointer.hpp>

namespace engine {

class ENGINE_API InputManager : public Singleton<InputManager> {
public:
    // Type definitions
    typedef std::function<void(SDL_Event*)> AppEventCallback;

    // All current touch screens.
    static const int kMaxSimultanuousPointers = 10;

    InputManager();

    ~InputManager();

    bool Initialize();

    void ShutDown();

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

    // Override standard Singleton retrieval.
    //     @remarks
    //         Why do we do this? Well, it's because the Singleton
    //         implementation is in a .h file, which means it gets compiled
    //         into anybody who includes it. This is needed for the
    //         Singleton template to work, but we actually only want it
    //         compiled into the implementation of the class based on the
    //         Singleton, not all of them. If we don't change this, we get
    //         link errors when trying to use the Singleton-based class from
    //         an outside dll.
    //     @par
    //         This method just delegates to the template version anyway,
    //         but the implementation stays in this single compilation unit,
    //         preventing link errors.
    static InputManager& GetInstance();

    // Override standard Singleton retrieval.
    //     @remarks
    //         Why do we do this? Well, it's because the Singleton
    //         implementation is in a .h file, which means it gets compiled
    //         into anybody who includes it. This is needed for the
    //         Singleton template to work, but we actually only want it
    //         compiled into the implementation of the class based on the
    //         Singleton, not all of them. If we don't change this, we get
    //         link errors when trying to use the Singleton-based class from
    //         an outside dll.
    //     @par
    //         This method just delegates to the template version anyway,
    //         but the implementation stays in this single compilation unit,
    //         preventing link errors.
    static InputManager* GetInstancePtr();

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
