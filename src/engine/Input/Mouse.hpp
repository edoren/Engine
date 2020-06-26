#pragma once

#include <Util/Prerequisites.hpp>

#include <Input/Pointer.hpp>
#include <Util/NonCopyable.hpp>

namespace engine {

class InputManager;

class ENGINE_API Mouse : NonCopyable {
    friend class InputManager;

private:
    Mouse(Pointer& pointer);

public:
    void setRelativeMouseMode(bool enable);

    void showCursor();

    void hideCursor();

    Pointer& pointer;
};

}  // namespace engine
