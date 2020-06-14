#include <Input/Button.hpp>

namespace engine {

Button::Button() : m_is_down(false) {
    advanceFrame();
}

bool Button::isDown() const {
    return m_is_down;
}

bool Button::wentDown() const {
    return m_went_down;
}

bool Button::wentUp() const {
    return m_went_up;
}

void Button::advanceFrame() {
    m_went_down = m_went_up = false;
}

void Button::update(bool down) {
    if (!m_is_down && down) {
        m_went_down = true;
    } else if (m_is_down && !down) {
        m_went_up = true;
    }
    m_is_down = down;
}

}  // namespace engine
