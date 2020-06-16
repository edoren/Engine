#include <Renderer/RenderStates.hpp>

namespace engine {

const RenderStates RenderStates::sDefault = RenderStates();

RenderStates::RenderStates() = default;

RenderStates::RenderStates(const RenderStates& other)

    = default;

RenderStates::RenderStates(RenderStates&& other)
      : transform(std::move(other.transform)),
        texture(std::move(other.texture)),
        shader(std::move(other.shader)) {
    other.texture = nullptr;
    other.shader = nullptr;
}

RenderStates& RenderStates::operator=(const RenderStates& other) {
    new (this) RenderStates(other);
    return *this;
}

RenderStates& RenderStates::operator=(RenderStates&& other) {
    new (this) RenderStates(std::move(other));
    return *this;
}

}  // namespace engine
