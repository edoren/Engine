#pragma once

#include <Util/Prerequisites.hpp>

#include <Renderer/Drawable.hpp>
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

    virtual void LoadFromData(
        std::vector<Vertex> vertices, std::vector<uint32> indices,
        std::vector<std::pair<Texture2D*, TextureType>> textures) = 0;

    virtual void Draw(RenderWindow& target,
                      const RenderStates& states) const = 0;

    void SetTexture(TextureType type, Texture2D* texture);

    const std::vector<Vertex>& GetVertices();
    const std::vector<uint32>& GetIndices();

protected:
    std::vector<Vertex> m_vertices;
    std::vector<uint32> m_indices;
    std::vector<std::pair<Texture2D*, TextureType>> m_textures;
    std::map<TextureType, Texture2D*> m_textures_map;
};

}  // namespace engine
