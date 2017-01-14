#include <Input/Button.hpp>

namespace engine {

Button::Button() : is_down_(false) {
    AdvanceFrame();
}

bool Button::IsDown() const {
    return is_down_;
}

bool Button::WentDown() const {
    return went_down_;
}

bool Button::WentUp() const {
    return went_up_;
}

void Button::AdvanceFrame() {
    went_down_ = went_up_ = false;
}

void Button::Update(bool down) {
    if (!is_down_ && down) {
        went_down_ = true;
    } else if (is_down_ && !down) {
        went_up_ = true;
    }
    is_down_ = down;
}

}  // namespace engine
