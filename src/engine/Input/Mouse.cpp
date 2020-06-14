#include <Input/Mouse.hpp>

#include <SDL2.h>

namespace engine {

Mouse::Mouse(Pointer& pointer) : pointer(pointer) {}

void Mouse::setRelativeMouseMode(bool enable) {
    if (enable) {
        SDL_SetRelativeMouseMode(SDL_TRUE);
    } else {
        SDL_SetRelativeMouseMode(SDL_FALSE);
    }
}

void Mouse::showCursor() {
    SDL_ShowCursor(1);
}
void Mouse::hideCursor() {
    SDL_ShowCursor(0);
}

}  // namespace engine
