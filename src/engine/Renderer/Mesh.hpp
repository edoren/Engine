#pragma once

#include <Util/Prerequisites.hpp>

#include <Renderer/RenderWindow.hpp>
#include <Renderer/TextureType.hpp>
#include <Renderer/Transform.hpp>
#include <Renderer/Vertex.hpp>

namespace engine {

class Texture2D;
class RenderStates;

class ENGINE_API Mesh {
public:
    Mesh();

    virtual ~Mesh();

    virtual void loadFromData(std::vector<Vertex> vertices,
                              std::vector<uint32> indices,
                              std::vector<std::pair<Texture2D*, TextureType>> textures) = 0;

    virtual void draw(RenderWindow& target, const RenderStates& states) const = 0;

    void setTexture(TextureType type, Texture2D* texture);

    const std::vector<Vertex>& getVertices();
    const std::vector<uint32>& getIndices();

protected:
    std::vector<Vertex> m_vertices;
    std::vector<uint32> m_indices;
    std::vector<std::pair<Texture2D*, TextureType>> m_textures;
    std::map<TextureType, Texture2D*> m_texturesMap;
};

}  // namespace engine
