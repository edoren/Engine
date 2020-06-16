#include <System/LogManager.hpp>
#include <System/StringFormat.hpp>

#include "VertexLayout.hpp"

namespace engine {

VertexLayout::VertexLayout() = default;

VertexLayout::VertexLayout(std::vector<VertexLayout::Component> vertex_input)
      : m_vertexInput(std::move(vertex_input)) {}

}  // namespace engine
