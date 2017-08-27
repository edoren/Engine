#pragma once

#include <Renderer/Mesh.hpp>
#include <Util/Prerequisites.hpp>

#include "GL_Config.hpp"

namespace engine {

class OPENGL_PLUGIN_API GL_Mesh : public Mesh {
public:
    GL_Mesh();

    ~GL_Mesh();

    void LoadFromData(
        std::vector<Vertex> vertices, std::vector<uint32> indices,
        std::vector<std::pair<Texture2D*, TextureType>> textures) override;

    void Draw() override;

private:
    void SetupMesh();

    unsigned int VAO, VBO, EBO;
};

}  // namespace engine
