#include <Input/InputManager.hpp>

#include <System/LogManager.hpp>
#include <System/StringFormat.hpp>
#include <System/StringView.hpp>

#include <SDL2.h>

namespace engine {

namespace {

const StringView sTag("InputManager");

int EngineEventFilter(void* userdata, SDL_Event* event) {
    ENGINE_UNUSED(userdata);
    ENGINE_UNUSED(event);
    // InputManager* input_system = reinterpret_cast<InputManager*>(userdata);
    // if (input_system->exit_requested()) {
    //     return 1;
    // }
    // switch (event->type) {
    //     default:
    //         break;
    // }
    return 1;
}

}  // namespace

template <>
InputManager* Singleton<InputManager>::sInstance = nullptr;

InputManager& InputManager::GetInstance() {
    return Singleton<InputManager>::GetInstance();
}

InputManager* InputManager::GetInstancePtr() {
    return Singleton<InputManager>::GetInstancePtr();
}

InputManager::InputManager()
      : m_exitRequested(false),
        m_pointers(sMaxSimultanuousPointers),
        m_mousewheelDelta(math::ivec2(0, 0)) {}

InputManager::~InputManager() {
    shutdown();
}

bool InputManager::initialize() {
    int status = SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER);
    SDL_SetEventFilter(EngineEventFilter, this);
    return status == 0;
}

void InputManager::shutdown() {
    SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER);
}

Button& InputManager::getButton(int button) {
    auto it = m_buttonMap.find(button);
    return it != m_buttonMap.end() ? it->second : (m_buttonMap[button] = Button());
}

Button& InputManager::getPointerButton(int64 pointer) {
    return getButton(static_cast<int>(pointer + SDLK_POINTER1));
}

void InputManager::advanceFrame() {
    // Reset our per-frame input state.
    m_mousewheelDelta.x = m_mousewheelDelta.y = 0;
    for (auto& button : m_buttonMap) {
        button.second.advanceFrame();
    }
    for (auto& pointer : m_pointers) {
        pointer.mousedelta.x = pointer.mousedelta.y = 0;
    }

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT: {
                m_exitRequested = true;
                break;
            }
            case SDL_APP_TERMINATING:
            case SDL_APP_LOWMEMORY:
                break;
            case SDL_APP_WILLENTERBACKGROUND: {
                onAppWillEnterBackground.emit();
                break;
            }
            case SDL_APP_DIDENTERBACKGROUND: {
                onAppDidEnterBackground.emit();
                break;
            }
            case SDL_APP_WILLENTERFOREGROUND: {
                onAppWillEnterForeground.emit();
                break;
            }
            case SDL_APP_DIDENTERFOREGROUND: {
                onAppDidEnterForeground.emit();
                break;
            }
            case SDL_WINDOWEVENT: {
                switch (event.window.event) {
                    case SDL_WINDOWEVENT_RESIZED: {
                        math::ivec2 windowSize(event.window.data1, event.window.data2);
                        onWindowResized.emit(windowSize);
                        break;
                    }
                    case SDL_WINDOWEVENT_MINIMIZED: {
                        onWindowMinimized.emit();
                        break;
                    }
                    case SDL_WINDOWEVENT_RESTORED: {
                        onWindowRestored.emit();
                        break;
                    }
                }
                break;
            }
            case SDL_SYSWMEVENT:
                break;
            case SDL_KEYDOWN:
            case SDL_KEYUP: {
                getButton(event.key.keysym.sym).update(event.key.state == SDL_PRESSED);
                break;
            }
            case SDL_TEXTEDITING:
            case SDL_TEXTINPUT:
            case SDL_KEYMAPCHANGED:
                break;
            case SDL_MOUSEMOTION: {
                m_pointers[0].mousedelta += math::ivec2(event.motion.xrel, event.motion.yrel);
                m_pointers[0].mousepos = math::ivec2(event.button.x, event.button.y);
                break;
            }
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP: {
                getPointerButton(event.button.button - 1).update(event.button.state == SDL_PRESSED);
                m_pointers[0].mousepos = math::ivec2(event.button.x, event.button.y);
                m_pointers[0].used = true;
                break;
            }
            case SDL_MOUSEWHEEL: {
                m_mousewheelDelta += math::ivec2(event.wheel.x, event.wheel.y);
                break;
            }
            case SDL_JOYAXISMOTION:
            case SDL_JOYBALLMOTION:
            case SDL_JOYHATMOTION:
            case SDL_JOYBUTTONDOWN:
            case SDL_JOYBUTTONUP:
            case SDL_JOYDEVICEADDED:
            case SDL_JOYDEVICEREMOVED:
            case SDL_CONTROLLERAXISMOTION:
            case SDL_CONTROLLERBUTTONDOWN:
            case SDL_CONTROLLERBUTTONUP:
            case SDL_CONTROLLERDEVICEADDED:
            case SDL_CONTROLLERDEVICEREMOVED:
            case SDL_CONTROLLERDEVICEREMAPPED:
            case SDL_FINGERDOWN:
            case SDL_FINGERUP:
            case SDL_FINGERMOTION:
            case SDL_DOLLARGESTURE:
            case SDL_DOLLARRECORD:
            case SDL_MULTIGESTURE:
            case SDL_CLIPBOARDUPDATE:
            case SDL_DROPFILE:
            case SDL_DROPTEXT:
            case SDL_DROPBEGIN:
            case SDL_DROPCOMPLETE:
            case SDL_AUDIODEVICEADDED:
            case SDL_AUDIODEVICEREMOVED:
            case SDL_RENDER_TARGETS_RESET:
            case SDL_RENDER_DEVICE_RESET:
            case SDL_USEREVENT:
                break;
            default: {
                LogDebug(sTag, "Unknown SDL Event ID: {}", event.type);
            }
        }
    }
}

}  // namespace engine
