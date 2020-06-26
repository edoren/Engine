#pragma once

#include <Util/Prerequisites.hpp>

#include <Renderer/Mesh.hpp>
#include <Util/Container/Vector.hpp>

#include "GL_Config.hpp"

#include <utility>

namespace engine {

class OPENGL_PLUGIN_API GL_Mesh : public Mesh {
public:
    GL_Mesh();

    ~GL_Mesh() override;

    void loadFromData(Vector<Vertex> vertices,
                      Vector<uint32> indices,
                      Vector<std::pair<Texture2D*, TextureType>> textures) override;

    void draw(RenderWindow& target, const RenderStates& states) const override;

private:
    void setupMesh();

    unsigned int m_vao, m_vbo, m_ebo;
};

}  // namespace engine
