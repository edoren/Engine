#pragma once

#include <Renderer/Texture2D.hpp>

#include "Vk_Buffer.hpp"
#include "Vk_Config.hpp"
#include "Vk_Dependencies.hpp"
#include "Vk_Image.hpp"

namespace engine {

class Vk_TextureManager;

class Image;

class Vk_Texture2D : public Texture2D {
public:
    Vk_Texture2D();

    ~Vk_Texture2D() override;

    bool LoadFromImage(const Image& img) override;

    void Use() override;

    VkDescriptorSet& GetDescriptorSet();

private:
    bool CreateSampler();

    bool CopyTextureData(const Image& img);

    bool AllocateDescriptorSet();
    bool UpdateDescriptorSet();

    Vk_Image m_image;
    VkSampler m_sampler;
    Vk_Buffer m_staging_buffer;
    VkDescriptorSet m_descriptor_set;
};

}  // namespace engine
