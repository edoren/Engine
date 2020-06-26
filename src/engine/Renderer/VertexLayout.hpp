#pragma once

#include <Util/Prerequisites.hpp>

#include <Util/Container/Vector.hpp>

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
    VertexLayout(Vector<Component> vertexInput);

protected:
    Vector<Component> m_vertexInput;
    Vector<Component> m_instanceInput;
};

}  // namespace engine
