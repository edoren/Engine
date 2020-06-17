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
    GL_CALL(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords)));
    // Vertex color coordinates
    GL_CALL(glEnableVertexAttribArray(3));
    GL_CALL(glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color)));

    GL_CALL(glBindVertexArray(0));
}

void GL_Mesh::draw(RenderWindow& target, const RenderStates& states) const {
    auto& window = static_cast<GL_RenderWindow&>(target);
    GL_Shader* shader = GL_ShaderManager::GetInstance().getActiveShader();

    uint32 diffuseNum = 1;
    uint32 specularNum = 1;
    for (size_t i = 0; i < m_textures.size(); i++) {
        const auto& pair = m_textures[i];
        auto* currentTexture = static_cast<GL_Texture2D*>(pair.first);
        TextureType currentTextureType = pair.second;

        String uniformName;
        switch (currentTextureType) {
            case TextureType::DIFFUSE:
                uniformName += "tex_diffuse" + String::FromValue(diffuseNum++);
                break;
            case TextureType::SPECULAR:
                uniformName += "tex_specular" + String::FromValue(specularNum++);
                break;
            default:
                continue;
        }

        GL_CALL(glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + i)));

        if (shader != nullptr) {
            shader->setUniform(uniformName, static_cast<GLint>(i));
        }

        if (currentTexture) {
            currentTexture->use();
        }
    }

    if (shader) {
        const Camera* activeCamera = window.getActiveCamera();

        math::mat4 modelMatrix = states.transform.getMatrix();

        math::mat4 viewMatrix = (activeCamera != nullptr) ? activeCamera->getViewMatrix() : math::mat4();
        const math::mat4& projectionMatrix = window.getProjectionMatrix();

        math::mat4 mvpMatrix = projectionMatrix * viewMatrix * modelMatrix;
        math::mat4 normalMatrix = modelMatrix.inverse().transpose();

        UniformBufferObject& uboDynamic = shader->getUboDynamic();
        uboDynamic.setAttributeValue("model", modelMatrix);
        uboDynamic.setAttributeValue("normalMatrix", normalMatrix);
        uboDynamic.setAttributeValue("mvp", mvpMatrix);
        shader->uploadUniformBuffers();
    }

    GL_CALL(glActiveTexture(GL_TEXTURE0));

    GL_CALL(glBindVertexArray(m_vao));
    GL_CALL(glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indices.size()), GL_UNSIGNED_INT, nullptr));
    GL_CALL(glBindVertexArray(0));
}

}  // namespace engine
