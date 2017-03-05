#pragma once

#include <Renderer/Renderer.hpp>

#include "Vk_Config.hpp"
#include "Vk_Dependencies.hpp"
#include "Vk_RenderWindow.hpp"

namespace engine {

class VULKAN_PLUGIN_API Vk_Renderer : public Renderer {
public:
    Vk_Renderer();

    virtual ~Vk_Renderer();

    virtual bool Initialize();

    virtual void Shutdown();

    virtual void AdvanceFrame();

    virtual Shader* CreateShader();  // TMP?

    virtual Texture2D* CreateTexture2D();  // TMP?
};

}  // namespace engine
