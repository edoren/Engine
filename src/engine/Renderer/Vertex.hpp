#pragma once

#include <Math/Math.hpp>
#include <System/StringFormat.hpp>
#include <Util/Prerequisites.hpp>

namespace engine {

struct ENGINE_API Vertex {
    Vertex() : position({0, 0, 0}), normal({0, 0, 0}), texCoords({0, 0}), color({0, 0, 0, 1}) {}
    Vertex(const math::vec3& position, const math::vec3& normal, const math::vec2& texcoords, const math::vec4& color)
          : position(position),
            normal(normal),
            texCoords(texcoords),
            color(color) {}

    math::Vector3Packed<float> position;
    math::Vector3Packed<float> normal;
    math::Vector2Packed<float> texCoords;
    math::Vector4Packed<float> color;
};

}  // namespace engine
