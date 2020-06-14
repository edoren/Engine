#pragma once

#include <Util/Prerequisites.hpp>

#include <Math/Math.hpp>

namespace engine {

class Color;

class ENGINE_API Color32 {
public:
    static const Color32 sBlack;
    static const Color32 sBlue;
    static const Color32 sCyan;
    static const Color32 sGray;
    static const Color32 sGreen;
    static const Color32 sMagenta;
    static const Color32 sRed;
    static const Color32 sTransparent;
    static const Color32 sWhite;
    static const Color32 sYellow;

    Color32();

    Color32(const Color32& color32);

    Color32(uint8 r, uint8 g, uint8 b, uint8 a);

    Color32(const math::Vector4<uint8>& vec);

    Color32(const Color& color);

    Color32& operator=(const Color32& color32);

    static Color32 Lerp(const Color32& a, const Color32& b, uint8 t);

    union {
        struct {
            uint8 r, g, b, a;
        };
        math::Vector4<uint8> data;
    };
};

}  // namespace engine
