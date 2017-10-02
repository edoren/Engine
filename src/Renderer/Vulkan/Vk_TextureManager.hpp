#pragma once

#include <Renderer/TextureManager.hpp>

#include "Vk_Config.hpp"
#include "Vk_Dependencies.hpp"

namespace engine {

class String;

class VULKAN_PLUGIN_API Vk_TextureManager : public TextureManager {
public:
    Vk_TextureManager();

    ~Vk_TextureManager();

protected:
    Texture2D* CreateTexture2D() override;

    void DeleteTexture2D(Texture2D* texture) override;

    void SetActiveTexture2D(Texture2D* texture) override;
};

}  // namespace engine
