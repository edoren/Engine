#pragma once

#include <Graphics/Color.hpp>
#include <Util/Precompiled.hpp>

namespace engine {

class Material {
public:
    Color ambient;
    Color diffuse;
    Color specular;
    float shininess;
};

}  // namespace engine
