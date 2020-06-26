#pragma once

#include <Util/Prerequisites.hpp>

#include <Renderer/RenderWindow.hpp>
#include <Renderer/TextureType.hpp>
#include <Renderer/Transform.hpp>
#include <Renderer/Vertex.hpp>
#include <Util/Container/Vector.hpp>

#include <map>
#include <utility>

namespace engine {

class Texture2D;
class RenderStates;

class ENGINE_API Mesh {
public:
    Mesh();

    virtual ~Mesh();

    virtual void loadFromData(Vector<Vertex> vertices,
                              Vector<uint32> indices,
                              Vector<std::pair<Texture2D*, TextureType>> textures) = 0;

    virtual void draw(RenderWindow& target, const RenderStates& states) const = 0;

    void setTexture(TextureType type, Texture2D* texture);

    const Vector<Vertex>& getVertices();
    const Vector<uint32>& getIndices();

protected:
    Vector<Vertex> m_vertices;
    Vector<uint32> m_indices;
    Vector<std::pair<Texture2D*, TextureType>> m_textures;
    std::map<TextureType, Texture2D*> m_texturesMap;
};

}  // namespace engine
