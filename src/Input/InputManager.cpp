#include <Input/InputManager.hpp>

#include <SDL.h>

namespace engine {

template <>
InputManager* Singleton<InputManager>::sInstance = nullptr;

InputManager& InputManager::GetInstance() {
    assert(sInstance);
    return (*sInstance);
}

InputManager* InputManager::GetInstancePtr() {
    return sInstance;
}

InputManager::InputManager()
      : m_exit_requested(false),
        m_pointers(kMaxSimultanuousPointers),
        m_mousewheel_delta(math::ivec2(0, 0)) {}

InputManager::~InputManager() {
    Shutdown();
}

bool InputManager::Initialize() {
    int status = SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER);
    SDL_SetEventFilter(HandleAppEvents, this);
    return status == 0;
}

void InputManager::Shutdown() {
    SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER);
}

int InputManager::HandleAppEvents(void* userdata, SDL_Event* event) {
    auto input_system = reinterpret_cast<InputManager*>(userdata);
    if (input_system->m_exit_requested) return 1;
    for (auto& callback : input_system->m_app_event_callbacks) {
        callback(event);
    }
    return 1;
}

Button& InputManager::GetButton(int button) {
    auto it = m_button_map.find(button);
    return it != m_button_map.end() ? it->second
                                   : (m_button_map[button] = Button());
}

Button& InputManager::GetPointerButton(SDL_FingerID pointer) {
    return GetButton(static_cast<int>(pointer + SDLK_POINTER1));
}

void InputManager::AddEventCallback(AppEventCallback callback) {
    m_app_event_callbacks.push_back(callback);
}

void InputManager::AdvanceFrame() {
    // Reset our per-frame input state.
    m_mousewheel_delta.x = m_mousewheel_delta.y = 0;
    for (auto& button : m_button_map) {
        button.second.AdvanceFrame();
    }
    for (auto& pointer : m_pointers) {
        pointer.mousedelta.x = pointer.mousedelta.y = 0;
    }

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT: {
                m_exit_requested = true;
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
                m_pointers[0].mousepos =
                    math::ivec2(event.button.x, event.button.y);
                m_pointers[0].used = true;
                break;
            }
            case SDL_MOUSEMOTION: {
                m_pointers[0].mousedelta +=
                    math::ivec2(event.motion.xrel, event.motion.yrel);
                m_pointers[0].mousepos =
                    math::ivec2(event.button.x, event.button.y);
                break;
            }
            case SDL_MOUSEWHEEL: {
                m_mousewheel_delta += math::ivec2(event.wheel.x, event.wheel.y);
                break;
            }
            case SDL_WINDOWEVENT: {
                switch (event.window.event) {
                    case SDL_WINDOWEVENT_RESIZED: {
                        math::ivec2 window_size(event.window.data1,
                                                event.window.data2);
                        OnWindowResized.Emit(window_size);
                        break;
                    }
                    case SDL_WINDOWEVENT_MINIMIZED: {
                        OnWindowMinimized.Emit();
                        break;
                    }
                    case SDL_WINDOWEVENT_RESTORED: {
                        OnWindowRestored.Emit();
                        break;
                    }
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
            case SDL_APP_WILLENTERBACKGROUND: {
                OnAppWillEnterBackground.Emit();
                // input_system->m_minimized_frame = input_system->m_frames;
                break;
            }
            case SDL_APP_DIDENTERBACKGROUND: {
                OnAppDidEnterBackground.Emit();
                break;
            }
            case SDL_APP_WILLENTERFOREGROUND: {
                OnAppWillEnterForeground.Emit();
                break;
            }
            case SDL_APP_DIDENTERFOREGROUND: {
                OnAppDidEnterForeground.Emit();
                // input_system->m_minimized_frame = input_system->m_frames;
                break;
            }
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
