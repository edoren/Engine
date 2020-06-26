#pragma once

#include <Renderer/TextureManager.hpp>

#include "Vk_Config.hpp"
#include "Vk_Dependencies.hpp"

#include <memory>

namespace engine {

class String;

class Vk_Texture2D;

class VULKAN_PLUGIN_API Vk_TextureManager : public TextureManager {
public:
    Vk_TextureManager();

    ~Vk_TextureManager() override;

    VkDescriptorPool& getDescriptorPool();

    VkDescriptorSetLayout& getDescriptorSetLayout();

    Vk_Texture2D* getActiveTexture2D();

    static Vk_TextureManager& GetInstance();

    static Vk_TextureManager* GetInstancePtr();

protected:
    std::unique_ptr<Texture2D> createTexture2D() override;
    void useTexture2D(Texture2D* texture) override;

private:
    bool createDescriptorPool();
    bool createDescriptorSetLayout();

    VkDescriptorPool m_descriptorPool;
    VkDescriptorSetLayout m_descriptorSetLayout;
};

}  // namespace engine
