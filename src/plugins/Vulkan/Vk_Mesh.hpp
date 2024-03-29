#pragma once

#include <Renderer/Mesh.hpp>
#include <Util/Container/Vector.hpp>

#include "Vk_Buffer.hpp"
#include "Vk_Config.hpp"
#include "Vk_Dependencies.hpp"

#include <utility>

namespace engine::plugin::vulkan {

class VULKAN_PLUGIN_API Vk_Mesh : public Mesh {
public:
    Vk_Mesh();

    ~Vk_Mesh() override;

    void loadFromData(Vector<Vertex> vertices,
                      Vector<uint32> indices,
                      Vector<std::pair<Texture2D*, TextureType>> textures) override;

    void draw(RenderWindow& target, const RenderStates& states) const override;

private:
    void setupMesh();

    Vk_Buffer m_vertexBuffer;
    Vk_Buffer m_indexBuffer;
};

}  // namespace engine::plugin::vulkan
