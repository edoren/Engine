#include "Renderer/Mesh.hpp"

#include <Util/Container/Vector.hpp>

namespace engine {

namespace {

const String sTag("Mesh");

}  // namespace

Mesh::Mesh() = default;

Mesh::~Mesh() = default;

const Vector<Vertex>& Mesh::getVertices() {
    return m_vertices;
}

const Vector<uint32>& Mesh::getIndices() {
    return m_indices;
}

}  // namespace engine
