#pragma once

#include <Util/Precompiled.hpp>

namespace engine {

class Color32;

class Color {
public:
    static const Color BLACK;
    static const Color BLUE;
    static const Color CYAN;
    static const Color GRAY;
    static const Color GREEN;
    static const Color MAGENTA;
    static const Color RED;
    static const Color TRANSPARENT;
    static const Color WHITE;
    static const Color YELLOW;

public:
    Color();

    Color(const Color& color);

    Color(float r, float g, float b, float a);

    Color(const math::vec4& vec);

    Color(const Color32& color32);

    Color& operator=(const Color& color);

    static Color Lerp(const Color& a, const Color& b, float t);

public:
    union {
        struct {
            float r, g, b, a;
        };
        math::Vector4<float> data;
    };
};

}  // namespace engine
