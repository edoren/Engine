#pragma once

#include <Util/Prerequisites.hpp>

#include <Math/Math.hpp>

namespace engine {

class Color32;

class ENGINE_API Color {
public:
    static const Color sBlack;
    static const Color sBlue;
    static const Color sCyan;
    static const Color sGray;
    static const Color sGreen;
    static const Color sMagenta;
    static const Color sRed;
    static const Color sTransparent;
    static const Color sWhite;
    static const Color sYellow;

    Color();

    Color(const Color& color);

    Color(float r, float g, float b, float a);

    Color(const math::vec4& vec);

    Color(const Color32& color32);

    Color& operator=(const Color& color);

    static Color Lerp(const Color& a, const Color& b, float t);

    union {
        struct {
            float r, g, b, a;
        };
        math::Vector4<float> data;
    };
};

}  // namespace engine
