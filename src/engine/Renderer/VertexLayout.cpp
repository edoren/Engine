#include <System/LogManager.hpp>
#include <System/StringFormat.hpp>

#include "VertexLayout.hpp"

namespace engine {

VertexLayout::VertexLayout() : m_vertex_input() {}

VertexLayout::VertexLayout(const std::vector<VertexLayout::Component>& vertex_input) : m_vertex_input(vertex_input) {}

VertexLayout::VertexLayout(std::vector<VertexLayout::Component>&& vertex_input)
      : m_vertex_input(std::move(vertex_input)) {}

}  // namespace engine
