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
    std::vector<Component> m_vertex_input;
    std::vector<Component> m_instance_input;
};

}  // namespace engine
