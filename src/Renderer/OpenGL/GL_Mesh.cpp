#include "GL_Mesh.hpp"

#include <Graphics/ResourceManager.hpp>
#include <System/LogManager.hpp>
#include <System/StringFormat.hpp>

#include "GL_Dependencies.hpp"
#include "GL_Shader.hpp"
#include "GL_Texture2D.hpp"
#include "GL_Utilities.hpp"

namespace engine {

GL_Mesh::GL_Mesh() : VAO(0), VBO(0), EBO(0) {}

GL_Mesh::~GL_Mesh() {
    if (EBO) {
        GL_CALL(glDeleteBuffers(1, &EBO));
        EBO = 0;
    }

    if (VBO) {
        GL_CALL(glDeleteBuffers(1, &VBO));
        VBO = 0;
    }

    if (VAO) {
        GL_CALL(glDeleteVertexArrays(1, &VAO));
        VAO = 0;
    }
}

void GL_Mesh::LoadFromData(
    std::vector<Vertex> vertices, std::vector<uint32> indices,
    std::vector<std::pair<Texture2D*, TextureType>> textures) {
    m_vertices = vertices;
    m_indices = indices;
    m_textures = textures;
    SetupMesh();
}

void GL_Mesh::SetupMesh() {
    GL_CALL(glGenVertexArrays(1, &VAO));

    GL_CALL(glGenBuffers(1, &VBO));
    GL_CALL(glGenBuffers(1, &EBO));

    GL_CALL(glBindVertexArray(VAO));

    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, VBO));
    GL_CALL(glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex),
                         m_vertices.data(), GL_STATIC_DRAW));

    GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO));
    GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                         m_indices.size() * sizeof(uint32), m_indices.data(),
                         GL_STATIC_DRAW));

    // Vertex positions
    GL_CALL(glEnableVertexAttribArray(0));
    GL_CALL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                                  (void*)offsetof(Vertex, m_position)));
    // Vertex normals
    GL_CALL(glEnableVertexAttribArray(1));
    GL_CALL(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                                  (void*)offsetof(Vertex, m_normal)));
    // Vertex texture coordinates
    GL_CALL(glEnableVertexAttribArray(2));
    GL_CALL(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                                  (void*)offsetof(Vertex, m_tex_coords)));

    GL_CALL(glBindVertexArray(0));
}

void GL_Mesh::Draw() {
    uint32 diffuse_num = 1;
    uint32 specular_num = 1;
    for (size_t i = 0; i < m_textures.size(); i++) {
        GL_CALL(glActiveTexture(GL_TEXTURE0 + i));

        String uniform_name = "material.";
        switch (m_textures[i].second) {
            case TextureType::eDiffuse:
                uniform_name += "tex_diffuse" + std::to_string(diffuse_num++);
                break;
            case TextureType::eSpecular:
                uniform_name += "tex_specular" + std::to_string(specular_num++);
                break;
            default:
                break;
        }

        ResourceManager& res = ResourceManager::GetInstance();

        // TODO: Add a ShaderManager to handle the current used shader
        GL_Shader* shader =
            reinterpret_cast<GL_Shader*>(res.FindShader("model"));
        GL_Texture2D* curr_texture =
            reinterpret_cast<GL_Texture2D*>(m_textures[i].first);

        if (shader) {
            shader->SetUniform(uniform_name, static_cast<GLint>(i));
        }

        if (curr_texture) {
            curr_texture->Use();
        }
    }
    GL_CALL(glActiveTexture(GL_TEXTURE0));

    GL_CALL(glBindVertexArray(VAO));
    GL_CALL(glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0));
    GL_CALL(glBindVertexArray(0));
}

}  // namespace engine
