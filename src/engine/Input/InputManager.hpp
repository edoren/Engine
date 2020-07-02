#pragma once

#include <Util/Prerequisites.hpp>

#include <Input/Button.hpp>
#include <Input/Mouse.hpp>
#include <Input/Pointer.hpp>
#include <Math/Math.hpp>
#include <Util/Container/Vector.hpp>
#include <Util/Singleton.hpp>

#include <System/Signal.hpp>

#include <map>
#include <memory>

union SDL_Event;

namespace engine {

class ENGINE_API InputManager : public Singleton<InputManager> {
public:
    // All current touch screens.
    static const int sMaxSimultanuousPointers = 10;

    InputManager();

    ~InputManager();

    bool initialize();

    void shutdown();

    void advanceFrame();

    Button& getButton(int button);

    Button& getPointerButton(int64 pointer);

    Mouse& getMouse() {
        static std::unique_ptr<Mouse> sMouseInstance(new Mouse(m_pointers[0]));
        return *sMouseInstance;
    }

    inline bool exitRequested() const {
        return m_exitRequested;
    }

    /**
     * @brief Override standard Singleton retrieval.
     *
     * @remarks Why do we do this? Well, it's because the Singleton
     *          implementation is in a .hpp file, which means it gets
     *          compiled into anybody who includes it. This is needed
     *          for the Singleton template to work, but we actually
     *          only compiled into the implementation of the class
     *          based on the Singleton, not all of them. If we don't
     *          change this, we get link errors when trying to use the
     *          Singleton-based class from an outside dll.
     *
     * @par This method just delegates to the template version anyway,
     *      but the implementation stays in this single compilation unit,
     *      preventing link errors.
     */
    static InputManager& GetInstance();

    /**
     * @brief Override standard Singleton retrieval.
     *
     * @remarks Why do we do this? Well, it's because the Singleton
     *          implementation is in a .hpp file, which means it gets
     *          compiled into anybody who includes it. This is needed
     *          for the Singleton template to work, but we actually
     *          only compiled into the implementation of the class
     *          based on the Singleton, not all of them. If we don't
     *          change this, we get link errors when trying to use the
     *          Singleton-based class from an outside dll.
     *
     * @par This method just delegates to the template version anyway,
     *      but the implementation stays in this single compilation unit,
     *      preventing link errors.
     */
    static InputManager* GetInstancePtr();

    Signal<const math::ivec2&> onWindowResized;
    Signal<> onWindowMinimized;
    Signal<> onWindowRestored;

    Signal<> onAppWillEnterBackground;
    Signal<> onAppDidEnterBackground;
    Signal<> onAppWillEnterForeground;
    Signal<> onAppDidEnterForeground;

private:
    bool m_exitRequested;
    Vector<Pointer> m_pointers;
    std::map<int, Button> m_buttonMap;
    math::ivec2 m_mousewheelDelta;
};

}  // namespace engine
