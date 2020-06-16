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
    RenderStates(RenderStates&& other) noexcept;

    RenderStates& operator=(const RenderStates& other);
    RenderStates& operator=(RenderStates&& other) noexcept;

    // Static member data
    static const RenderStates sDefault;

    // Member data
    Transform transform;
    const Texture2D* texture;
    const Shader* shader;
};

}  // namespace engine
