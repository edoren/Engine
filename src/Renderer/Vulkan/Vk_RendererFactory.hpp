#pragma once

#include <Renderer/RendererFactory.hpp>

#include "Vk_Config.hpp"

namespace engine {

class VULKAN_PLUGIN_API Vk_RendererFactory : public RendererFactory {
public:
    Vk_RendererFactory();

    ~Vk_RendererFactory();

    Shader* CreateShader() override;

    Texture2D* CreateTexture2D() override;

    Mesh* CreateMesh() override;
};

}  // namespace engine
