#include <System/LogManager.hpp>
#include <System/StringFormat.hpp>
#include <Util/Container/Vector.hpp>

#include "VertexLayout.hpp"

namespace engine {

VertexLayout::VertexLayout() = default;

VertexLayout::VertexLayout(Vector<VertexLayout::Component> vertexInput) : m_vertexInput(std::move(vertexInput)) {}

}  // namespace engine
