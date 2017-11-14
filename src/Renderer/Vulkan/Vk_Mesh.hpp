#pragma once

#include <Renderer/Mesh.hpp>

#include "Vk_Buffer.hpp"
#include "Vk_Config.hpp"
#include "Vk_Dependencies.hpp"

namespace engine {

class VULKAN_PLUGIN_API Vk_Mesh : public Mesh {
public:
    Vk_Mesh();

    ~Vk_Mesh() override;

    void LoadFromData(
        std::vector<Vertex> vertices, std::vector<uint32> indices,
        std::vector<std::pair<Texture2D*, TextureType>> textures) override;

    void Draw() const override;

private:
    void SetupMesh();

    Vk_Buffer m_vertex_buffer;
    Vk_Buffer m_index_buffer;
};

}  // namespace engine
