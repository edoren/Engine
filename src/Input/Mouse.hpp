#pragma once

#include <Util/Precompiled.hpp>
#include <Input/Pointer.hpp>

namespace engine {

class InputManager;

class ENGINE_API Mouse : NonCopyable {
    friend class InputManager;

private:
    Mouse(Pointer& pointer);

public:
    void SetRelativeMouseMode(bool enable);

    void ShowCursor();

    void HideCursor();

    Pointer& pointer;
};

}  // namespace engine
