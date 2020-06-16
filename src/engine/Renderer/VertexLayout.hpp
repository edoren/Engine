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
    VertexLayout(std::vector<Component> vertexInput);

protected:
    std::vector<Component> m_vertexInput;
    std::vector<Component> m_instanceInput;
};

}  // namespace engine
