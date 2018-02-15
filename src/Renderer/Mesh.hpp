#pragma once

#include <Util/Prerequisites.hpp>

#include <Renderer/Drawable.hpp>
#include <Renderer/TextureType.hpp>
#include <Renderer/Vertex.hpp>

namespace engine {

class Texture2D;

class ENGINE_API Mesh : public Drawable {
public:
    Mesh();

    virtual ~Mesh();

    virtual void LoadFromData(
        std::vector<Vertex> vertices, std::vector<uint32> indices,
        std::vector<std::pair<Texture2D*, TextureType>> textures) = 0;

    virtual void Draw(RenderWindow& target) const = 0;

protected:
    std::vector<Vertex> m_vertices;
    std::vector<uint32> m_indices;
    std::vector<std::pair<Texture2D*, TextureType>> m_textures;
};

}  // namespace engine
