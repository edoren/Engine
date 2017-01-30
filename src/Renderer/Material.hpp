#pragma once

#include <Util/Prerequisites.hpp>

#include <Graphics/Color.hpp>

namespace engine {

class ENGINE_API Material {
public:
    Color ambient;
    Color diffuse;
    Color specular;
    float shininess;
};

}  // namespace engine
