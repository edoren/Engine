#pragma once

#include <Renderer/TextureManager.hpp>

#include "Vk_Config.hpp"
#include "Vk_Dependencies.hpp"

namespace engine {

class String;

class Vk_Texture2D;

class VULKAN_PLUGIN_API Vk_TextureManager : public TextureManager {
public:
    Vk_TextureManager();

    ~Vk_TextureManager();

    VkDescriptorPool& GetDescriptorPool();

    VkDescriptorSetLayout& GetDescriptorSetLayout();

    Vk_Texture2D* GetActiveTexture2D();

    static Vk_TextureManager& GetInstance();

    static Vk_TextureManager* GetInstancePtr();

protected:
    Texture2D* CreateTexture2D() override;

    void DeleteTexture2D(Texture2D* texture) override;

    void UseTexture2D(Texture2D* texture) override;

private:
    bool CreateDescriptorPool();
    bool CreateDescriptorSetLayout();

    VkDescriptorPool m_descriptor_pool;
    VkDescriptorSetLayout m_descriptor_set_layout;
};

}  // namespace engine
