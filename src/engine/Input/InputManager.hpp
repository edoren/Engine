#pragma once

#include <Input/Button.hpp>
#include <Input/Mouse.hpp>
#include <Input/Pointer.hpp>
#include <Math/Math.hpp>
#include <Util/Prerequisites.hpp>

#include <System/Signal.hpp>

union SDL_Event;

namespace engine {

class ENGINE_API InputManager : public Singleton<InputManager> {
public:
    // All current touch screens.
    static const int sMaxSimultanuousPointers = 10;

    InputManager();

    ~InputManager();

    bool Initialize();

    void Shutdown();

    void AdvanceFrame();

    Button& GetButton(int button);

    Button& GetPointerButton(int64 pointer);

    Mouse& GetMouse() {
        static std::unique_ptr<Mouse> instance = std::make_unique<Mouse>(m_pointers[0]);
        return *instance;
    }

    inline bool exit_requested() const {
        return m_exit_requested;
    }

    ////////////////////////////////////////////////////////////
    /// @brief Override standard Singleton retrieval.
    ///
    /// @remarks Why do we do this? Well, it's because the Singleton
    ///          implementation is in a .hpp file, which means it gets
    ///          compiled into anybody who includes it. This is needed
    ///          for the Singleton template to work, but we actually
    ///          only compiled into the implementation of the class
    ///          based on the Singleton, not all of them. If we don't
    ///          change this, we get link errors when trying to use the
    ///          Singleton-based class from an outside dll.
    ///
    /// @par This method just delegates to the template version anyway,
    ///      but the implementation stays in this single compilation unit,
    ///      preventing link errors.
    ////////////////////////////////////////////////////////////
    static InputManager& GetInstance();

    ////////////////////////////////////////////////////////////
    /// @brief Override standard Singleton retrieval.
    ///
    /// @remarks Why do we do this? Well, it's because the Singleton
    ///          implementation is in a .hpp file, which means it gets
    ///          compiled into anybody who includes it. This is needed
    ///          for the Singleton template to work, but we actually
    ///          only compiled into the implementation of the class
    ///          based on the Singleton, not all of them. If we don't
    ///          change this, we get link errors when trying to use the
    ///          Singleton-based class from an outside dll.
    ///
    /// @par This method just delegates to the template version anyway,
    ///      but the implementation stays in this single compilation unit,
    ///      preventing link errors.
    ////////////////////////////////////////////////////////////
    static InputManager* GetInstancePtr();

    Signal<const math::ivec2&> OnWindowResized;
    Signal<> OnWindowMinimized;
    Signal<> OnWindowRestored;

    Signal<> OnAppWillEnterBackground;
    Signal<> OnAppDidEnterBackground;
    Signal<> OnAppWillEnterForeground;
    Signal<> OnAppDidEnterForeground;

private:
    bool m_exit_requested;
    std::vector<Pointer> m_pointers;
    std::map<int, Button> m_button_map;
    math::ivec2 m_mousewheel_delta;
};

}  // namespace engine
