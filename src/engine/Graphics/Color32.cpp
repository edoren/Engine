#include <Graphics/Color.hpp>
#include <Graphics/Color32.hpp>

namespace engine {

const Color32 Color32::sBlack(0, 0, 0, 255);
const Color32 Color32::sBlue(0, 0, 255, 255);
const Color32 Color32::sCyan(0, 255, 255, 255);
const Color32 Color32::sGray(127, 127, 127, 255);
const Color32 Color32::sGreen(0, 255, 0, 255);
const Color32 Color32::sMagenta(255, 0, 255, 255);
const Color32 Color32::sRed(255, 0, 0, 255);
const Color32 Color32::sTransparent(0, 0, 0, 0);
const Color32 Color32::sWhite(255, 255, 255, 255);
const Color32 Color32::sYellow(255, 235, 4, 255);

Color32::Color32() : r(0), g(0), b(0), a(255) {}

Color32::Color32(const Color32& color) : r(color.r), g(color.g), b(color.b), a(color.a) {}

Color32::Color32(uint8 r, uint8 g, uint8 b, uint8 a) : r(r), g(g), b(b), a(a) {}

Color32::Color32(const math::Vector4<uint8>& vec) : m_data(vec) {}

Color32::Color32(const Color& color)
      : r(static_cast<uint8>(color.r * 255.F)),
        g(static_cast<uint8>(color.g * 255.F)),
        b(static_cast<uint8>(color.b * 255.F)),
        a(static_cast<uint8>(color.a * 255.F)) {}

Color32& Color32::operator=(const Color32& color) {
    m_data = color.m_data;
    return *this;
};

Color32 Color32::Lerp(const Color32& a, const Color32& b, uint8 t) {
    return Color32(math::Vector4<uint8>::Lerp(a.m_data, b.m_data, t));
}

}  // namespace engine
