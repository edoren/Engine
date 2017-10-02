#pragma once

#include <Renderer/TextureManager.hpp>

#include "Vk_Config.hpp"
#include "Vk_Dependencies.hpp"

namespace engine {

class String;

class VULKAN_PLUGIN_API Vk_TextureManagerDelegate
    : public TextureManagerDelegate {
public:
    Vk_TextureManagerDelegate();

    ~Vk_TextureManagerDelegate();

    Texture2D* CreateTexture2D() override;

    void DeleteTexture2D(Texture2D* texture) override;

    void SetActiveTexture2D(Texture2D* texture) override;
};

}  // namespace engine
