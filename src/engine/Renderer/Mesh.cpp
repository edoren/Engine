#include "Renderer/Mesh.hpp"

namespace engine {

namespace {

const String sTag("Mesh");

}  // namespace

Mesh::Mesh() {}

Mesh::~Mesh() {}

const std::vector<Vertex>& Mesh::getVertices() {
    return m_vertices;
}

const std::vector<uint32>& Mesh::getIndices() {
    return m_indices;
}

}  // namespace engine
