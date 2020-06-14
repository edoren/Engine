#include <Input/Button.hpp>

namespace engine {

Button::Button() : m_isDown(false) {
    advanceFrame();
}

bool Button::isDown() const {
    return m_isDown;
}

bool Button::wentDown() const {
    return m_wentDown;
}

bool Button::wentUp() const {
    return m_wentUp;
}

void Button::advanceFrame() {
    m_wentDown = m_wentUp = false;
}

void Button::update(bool down) {
    if (!m_isDown && down) {
        m_wentDown = true;
    } else if (m_isDown && !down) {
        m_wentUp = true;
    }
    m_isDown = down;
}

}  // namespace engine
