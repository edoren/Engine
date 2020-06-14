#pragma once

#include <Util/Prerequisites.hpp>

namespace engine {

class ENGINE_API VertexLayout {
public:
    enum class Component {
        POSITION,
        NORMAL,
        UV,
        COLOR,
    };

    VertexLayout();
    VertexLayout(const std::vector<Component>& vertex_input);
    VertexLayout(std::vector<Component>&& vertex_input);

protected:
    std::vector<Component> m_vertexInput;
    std::vector<Component> m_instanceInput;
};

}  // namespace engine
