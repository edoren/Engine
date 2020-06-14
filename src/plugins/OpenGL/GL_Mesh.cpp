#include "GL_Mesh.hpp"

#include <Graphics/3D/Camera.hpp>
#include <System/LogManager.hpp>
#include <Renderer/RenderStates.hpp>
#include <System/StringFormat.hpp>

#include "GL_Dependencies.hpp"
#include "GL_RenderWindow.hpp"
#include "GL_Shader.hpp"
#include "GL_ShaderManager.hpp"
#include "GL_Texture2D.hpp"
#include "GL_Utilities.hpp"

namespace engine {

GL_Mesh::GL_Mesh() : m_vao(0), m_vbo(0), m_ebo(0) {}

GL_Mesh::~GL_Mesh() {
    GL_CALL(glBindVertexArray(0));

    if (m_ebo) {
        GL_CALL(glDeleteBuffers(1, &m_ebo));
        m_ebo = 0;
    }

    if (m_vbo) {
        GL_CALL(glDeleteBuffers(1, &m_vbo));
        m_vbo = 0;
    }

    if (m_vao) {
        GL_CALL(glDeleteVertexArrays(1, &m_vao));
        m_vao = 0;
    }
}

void GL_Mesh::loadFromData(std::vector<Vertex> vertices,
                           std::vector<uint32> indices,
                           std::vector<std::pair<Texture2D*, TextureType>> textures) {
    m_vertices = std::move(vertices);
    m_indices = std::move(indices);
    m_textures = std::move(textures);
    setupMesh();
}

void GL_Mesh::setupMesh() {
    GL_CALL(glGenVertexArrays(1, &m_vao));

    GL_CALL(glGenBuffers(1, &m_vbo));
    GL_CALL(glGenBuffers(1, &m_ebo));

    GL_CALL(glBindVertexArray(m_vao));

    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, m_vbo));
    GL_CALL(glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex), m_vertices.data(), GL_STATIC_DRAW));

    GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo));
    GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(uint32), m_indices.data(), GL_STATIC_DRAW));

    // Vertex positions
    GL_CALL(glEnableVertexAttribArray(0));
    GL_CALL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position)));
    // Vertex normals
    GL_CALL(glEnableVertexAttribArray(1));
    GL_CALL(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal)));
    // Vertex texture coordinates
    GL_CALL(glEnableVertexAttribArray(2));
    GL_CALL(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tex_coords)));
    // Vertex color coordinates
    GL_CALL(glEnableVertexAttribArray(3));
    GL_CALL(glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color)));

    GL_CALL(glBindVertexArray(0));
}

void GL_Mesh::draw(RenderWindow& target, const RenderStates& states) const {
    GL_RenderWindow& window = static_cast<GL_RenderWindow&>(target);
    GL_Shader* shader = GL_ShaderManager::GetInstance().getActiveShader();

    uint32 diffuse_num = 1;
    uint32 specular_num = 1;
    for (size_t i = 0; i < m_textures.size(); i++) {
        const auto& pair = m_textures[i];
        GL_Texture2D* current_texture = static_cast<GL_Texture2D*>(pair.first);
        TextureType current_texture_type = pair.second;

        String uniform_name;
        switch (current_texture_type) {
            case TextureType::DIFFUSE:
                uniform_name += "tex_diffuse" + String::FromValue(diffuse_num++);
                break;
            case TextureType::SPECULAR:
                uniform_name += "tex_specular" + String::FromValue(specular_num++);
                break;
            default:
                continue;
        }

        GL_CALL(glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + i)));

        if (shader != nullptr) {
            shader->setUniform(uniform_name, static_cast<GLint>(i));
        }

        if (current_texture) {
            current_texture->use();
        }
    }

    if (shader) {
        const Camera* active_camera = window.getActiveCamera();

        math::mat4 model_matrix = states.transform.getMatrix();

        math::mat4 view_matrix = (active_camera != nullptr) ? active_camera->getViewMatrix() : math::mat4();
        const math::mat4& projection_matrix = window.getProjectionMatrix();

        math::mat4 mvp_matrix = projection_matrix * view_matrix * model_matrix;
        math::mat4 normal_matrix = model_matrix.inverse().transpose();

        UniformBufferObject& ubo_dynamic = shader->getUboDynamic();
        ubo_dynamic.setAttributeValue("model", model_matrix);
        ubo_dynamic.setAttributeValue("normalMatrix", normal_matrix);
        ubo_dynamic.setAttributeValue("mvp", mvp_matrix);
        shader->uploadUniformBuffers();
    }

    GL_CALL(glActiveTexture(GL_TEXTURE0));

    GL_CALL(glBindVertexArray(m_vao));
    GL_CALL(glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indices.size()), GL_UNSIGNED_INT, 0));
    GL_CALL(glBindVertexArray(0));
}

}  // namespace engine
