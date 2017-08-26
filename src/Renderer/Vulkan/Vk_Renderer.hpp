#pragma once

#include <Renderer/Renderer.hpp>

#include "Vk_Config.hpp"
#include "Vk_Context.hpp"
#include "Vk_RenderWindow.hpp"

namespace engine {

class VULKAN_PLUGIN_API Vk_Renderer : public Renderer {
public:
    Vk_Renderer();

    ~Vk_Renderer();

    bool Initialize() override;

    void Shutdown() override;

    void AdvanceFrame() override;

    // TODO: Change this for a factory
    Shader* CreateShader() override;
    Texture2D* CreateTexture2D() override;
    // Mesh* CreateMesh() override;

private:
    Vk_Context* m_context;
};

}  // namespace engine
