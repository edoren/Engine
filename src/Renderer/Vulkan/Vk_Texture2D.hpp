#pragma once

#include <Renderer/Texture2D.hpp>

#include "Vk_Buffer.hpp"
#include "Vk_Config.hpp"
#include "Vk_Dependencies.hpp"
#include "Vk_VulkanParameters.hpp"

namespace engine {

class Vk_TextureManager;

class Image;

class Vk_Texture2D : public Texture2D {
public:
    Vk_Texture2D(Vk_TextureManager* texture_manager);

    virtual ~Vk_Texture2D();

    virtual bool LoadFromImage(const Image& img);

    virtual void Use();

    VkDescriptorSet& GetDescriptorSet();

private:
    bool CreateImage(const Image& img);
    bool CreateImageView();
    bool CreateSampler();

    bool CopyTextureData(const Image& img);

    bool AllocateDescriptorSet();
    bool UpdateDescriptorSet();

    Vk_TextureManager* m_texture_manager;

    ImageParameters m_image;
    VkDeviceMemory m_memory;
    VkSampler m_sampler;
    Vk_Buffer m_staging_buffer;
    VkDescriptorSet m_descriptor_set;
};

}  // namespace engine
