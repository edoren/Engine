#pragma once

#include <Util/Prerequisites.hpp>

#include <Input/Pointer.hpp>

namespace engine {

class InputManager;

class ENGINE_API Mouse : NonCopyable {
    friend class InputManager;
    friend std::unique_ptr<Mouse> std::make_unique<Mouse>(Pointer& pointer);

private:
    Mouse(Pointer& pointer);

public:
    void setRelativeMouseMode(bool enable);

    void showCursor();

    void hideCursor();

    Pointer& pointer;
};

}  // namespace engine
