#pragma once

#include <Renderer/Mesh.hpp>
#include <Util/Prerequisites.hpp>

#include "GL_Config.hpp"

namespace engine {

class OPENGL_PLUGIN_API GL_Mesh : public Mesh {
public:
    GL_Mesh();

    ~GL_Mesh() override;

    void loadFromData(std::vector<Vertex> vertices,
                      std::vector<uint32> indices,
                      std::vector<std::pair<Texture2D*, TextureType>> textures) override;

    void draw(RenderWindow& target, const RenderStates& states) const override;

private:
    void setupMesh();

    unsigned int m_vao, m_vbo, m_ebo;
};

}  // namespace engine
