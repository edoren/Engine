#include <Input/Mouse.hpp>

namespace engine {

Mouse::Mouse(Pointer& pointer) : pointer(pointer) {}

void Mouse::SetRelativeMouseMode(bool enable) {
    if (enable) {
        SDL_SetRelativeMouseMode(SDL_TRUE);
    } else {
        SDL_SetRelativeMouseMode(SDL_FALSE);
    }
}

void Mouse::ShowCursor() {
    SDL_ShowCursor(1);
}
void Mouse::HideCursor() {
    SDL_ShowCursor(0);
}

}  // namespace engine
