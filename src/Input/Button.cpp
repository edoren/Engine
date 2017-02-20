#include <Input/Button.hpp>

namespace engine {

Button::Button() : m_is_down(false) {
    AdvanceFrame();
}

bool Button::IsDown() const {
    return m_is_down;
}

bool Button::WentDown() const {
    return m_went_down;
}

bool Button::WentUp() const {
    return m_went_up;
}

void Button::AdvanceFrame() {
    m_went_down = m_went_up = false;
}

void Button::Update(bool down) {
    if (!m_is_down && down) {
        m_went_down = true;
    } else if (m_is_down && !down) {
        m_went_up = true;
    }
    m_is_down = down;
}

}  // namespace engine
