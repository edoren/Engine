#include <Graphics/Color.hpp>
#include <Graphics/Color32.hpp>

namespace engine {

const Color Color::BLACK(0.0f, 0.0f, 0.0f, 1.0f);
const Color Color::BLUE(0.0f, 0.0f, 1.0f, 1.0f);
const Color Color::CYAN(0.0f, 1.0f, 1.0f, 1.0f);
const Color Color::GRAY(0.5f, 0.5f, 0.5f, 1.0f);
const Color Color::GREEN(0.0f, 1.0f, 0.0f, 1.0f);
const Color Color::MAGENTA(1.0f, 0.0f, 1.0f, 1.0f);
const Color Color::RED(1.0f, 0.0f, 0.0f, 1.0f);
const Color Color::TRANSPARENT(0.0f, 0.0f, 0.0f, 0.0f);
const Color Color::WHITE(1.0f, 1.0f, 1.0f, 1.0f);
const Color Color::YELLOW(1.0f, 0.92f, 0.016f, 1.0f);

Color::Color() : r(0), g(0), b(0), a(1) {};

Color::Color(const Color& color)
      : r(color.r), g(color.g), b(color.b), a(color.a) {};

Color::Color(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {}

Color::Color(const math::vec4& vec) : m_data(vec) {}

Color::Color(const Color32& color32)
      : r(color32.r / 255.f),
        g(color32.g / 255.f),
        b(color32.b / 255.f),
        a(color32.a / 255.f) {}

Color& Color::operator=(const Color& color) {
    m_data = color.m_data;
    return *this;
};

Color Color::Lerp(const Color& a, const Color& b, float t) {
    return Color(math::Vector4<float>::Lerp(a.m_data, b.m_data, t));
}

}  // namespace engine
