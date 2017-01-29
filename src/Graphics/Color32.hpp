#pragma once

#include <Util/Precompiled.hpp>

namespace engine {

class Color;

class ENGINE_API Color32 {
public:
    static const Color32 BLACK;
    static const Color32 BLUE;
    static const Color32 CYAN;
    static const Color32 GRAY;
    static const Color32 GREEN;
    static const Color32 MAGENTA;
    static const Color32 RED;
    static const Color32 TRANSPARENT;
    static const Color32 WHITE;
    static const Color32 YELLOW;

public:
    Color32();

    Color32(const Color32& color32);

    Color32(uint8 r, uint8 g, uint8 b, uint8 a);

    Color32(const math::Vector4<uint8>& vec);

    Color32(const Color& color);

    Color32& operator=(const Color32& color32);

    static Color32 Lerp(const Color32& a, const Color32& b, uint8 t);

public:
    union {
        struct {
            uint8 r, g, b, a;
        };
        math::Vector4<uint8> data_;
    };
};

}  // namespace engine
