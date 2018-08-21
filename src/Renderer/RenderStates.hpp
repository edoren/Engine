#pragma once

#include <Util/Prerequisites.hpp>

#include <Renderer/Transform.hpp>

namespace engine {

class Texture2D;
class Shader;

class ENGINE_API RenderStates {
public:
    RenderStates();
    RenderStates(const RenderStates& other);
    RenderStates(RenderStates&& other);

    RenderStates& operator=(const RenderStates& other);
    RenderStates& operator=(RenderStates&& other);

    // Static member data
    static const RenderStates Default;

    // Member data
    Transform transform;
    const Texture2D* texture;
    const Shader* shader;
};

}  // namespace engine
