#include <Input/InputManager.hpp>

namespace engine {

template <>
InputManager* Singleton<InputManager>::s_instance = nullptr;

InputManager& InputManager::GetInstance() {
    assert(s_instance);
    return (*s_instance);
}

InputManager* InputManager::GetInstancePtr() {
    return s_instance;
}

InputManager::InputManager()
      : minimized_(false),
        exit_requested_(false),
        pointers_(kMaxSimultanuousPointers),
        mousewheel_delta_(math::ivec2(0, 0)) {}

InputManager::~InputManager() {
    ShutDown();
}

bool InputManager::Initialize() {
    int status = SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER);
    SDL_SetEventFilter(HandleAppEvents, this);
    return status == 0;
}

void InputManager::ShutDown() {
    SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER);
}

int InputManager::HandleAppEvents(void* userdata, SDL_Event* event) {
    auto input_system = reinterpret_cast<InputManager*>(userdata);
    if (input_system->exit_requested_) return 1;
    for (auto& callback : input_system->app_event_callbacks_) {
        callback(event);
    }
    return 1;
}

Button& InputManager::GetButton(int button) {
    auto it = button_map_.find(button);
    return it != button_map_.end() ? it->second
                                   : (button_map_[button] = Button());
}

Button& InputManager::GetPointerButton(SDL_FingerID pointer) {
    return GetButton(static_cast<int>(pointer + SDLK_POINTER1));
}

void InputManager::AddEventCallback(AppEventCallback callback) {
    app_event_callbacks_.push_back(callback);
}

void InputManager::AdvanceFrame(math::ivec2* window_size) {
    // Reset our per-frame input state.
    mousewheel_delta_.x = mousewheel_delta_.y = 0;
    for (auto& button : button_map_) {
        button.second.AdvanceFrame();
    }
    for (auto& pointer : pointers_) {
        pointer.mousedelta.x = pointer.mousedelta.y = 0;
    }

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT: {
                exit_requested_ = true;
                break;
            }
            case SDL_KEYDOWN:
            case SDL_KEYUP: {
                GetButton(event.key.keysym.sym)
                    .Update(event.key.state == SDL_PRESSED);
                break;
            }
            // These fire from e.g. OS X touchpads. Ignore them because we just
            // want the mouse events.
            case SDL_FINGERDOWN:
                break;
            case SDL_FINGERUP:
                break;
            case SDL_FINGERMOTION:
                break;
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP: {
                GetPointerButton(event.button.button - 1)
                    .Update(event.button.state == SDL_PRESSED);
                pointers_[0].mousepos =
                    math::ivec2(event.button.x, event.button.y);
                pointers_[0].used = true;
                break;
            }
            case SDL_MOUSEMOTION: {
                pointers_[0].mousedelta +=
                    math::ivec2(event.motion.xrel, event.motion.yrel);
                pointers_[0].mousepos =
                    math::ivec2(event.button.x, event.button.y);
                break;
            }
            case SDL_MOUSEWHEEL: {
                mousewheel_delta_ += math::ivec2(event.wheel.x, event.wheel.y);
                break;
            }
            case SDL_WINDOWEVENT: {
                switch (event.window.event) {
                    case SDL_WINDOWEVENT_RESIZED:
                        *window_size =
                            math::ivec2(event.window.data1, event.window.data2);
                        break;
                    case SDL_WINDOWEVENT_MINIMIZED:
                        minimized_ = true;
                        break;
                    case SDL_WINDOWEVENT_RESTORED:
                        minimized_ = false;
                        break;
                }
                break;
            }
            case SDL_TEXTEDITING:
            case SDL_TEXTINPUT:
                break;
            case SDL_APP_TERMINATING:
                break;
            case SDL_APP_LOWMEMORY:
                break;
            case SDL_APP_WILLENTERBACKGROUND:
                minimized_ = true;
                // input_system->minimized_frame_ = input_system->frames_;
                break;
            case SDL_APP_DIDENTERBACKGROUND:
                break;
            case SDL_APP_WILLENTERFOREGROUND:
                break;
            case SDL_APP_DIDENTERFOREGROUND:
                minimized_ = false;
                // input_system->minimized_frame_ = input_system->frames_;
                break;
            default: {
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                            "----Unknown SDL event!\n");
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "----Event ID: %d!\n",
                            event.type);
            }
        }
    }
}

}  // namespace engine
