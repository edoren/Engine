#include <System/LogManager.hpp>
#include <System/StringFormat.hpp>

#include "VertexLayout.hpp"

namespace engine {

VertexLayout::VertexLayout() = default;

VertexLayout::VertexLayout(std::vector<VertexLayout::Component> vertexInput) : m_vertexInput(std::move(vertexInput)) {}

}  // namespace engine
