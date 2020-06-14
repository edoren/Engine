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

    bool loadFromImage(const Image& img) override;

    void use() override;

    VkDescriptorSet& getDescriptorSet();

private:
    bool createSampler();

    bool copyTextureData(const Image& img);

    bool allocateDescriptorSet();
    bool updateDescriptorSet();

    Vk_Image m_image;
    VkSampler m_sampler;
    Vk_Buffer m_stagingBuffer;
    VkDescriptorSet m_descriptorSet;
};

}  // namespace engine
