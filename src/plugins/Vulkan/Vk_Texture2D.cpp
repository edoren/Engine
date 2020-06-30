#include <System/LogManager.hpp>
#include <System/StringFormat.hpp>
#include <System/StringView.hpp>

#include "Vk_Context.hpp"
#include "Vk_Texture2D.hpp"
#include "Vk_TextureManager.hpp"
#include "Vk_Utilities.hpp"

namespace engine {

namespace {

const StringView sTag("Vk_Texture2D");

}  // namespace

Vk_Texture2D::Vk_Texture2D() = default;

Vk_Texture2D::~Vk_Texture2D() {
    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.getVulkanDevice();
    QueueParameters& graphicsQueue = context.getGraphicsQueue();

    vkQueueWaitIdle(graphicsQueue.handle);

    if (m_sampler != VK_NULL_HANDLE) {
        vkDestroySampler(device, m_sampler, nullptr);
        m_sampler = VK_NULL_HANDLE;
    }
}

bool Vk_Texture2D::loadFromImage(const Image& img) {
    if (!m_image.createImage(img.getSize(), VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
                             (VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT))) {
        LogError(sTag, "Could not create image");
        return false;
    }

    if (!m_image.allocateMemory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
        LogError(sTag, "Could not allocate memory for image");
        return false;
    }

    if (!m_image.createImageView(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT)) {
        LogError(sTag, "Could not create image view");
        return false;
    }

    if (!createSampler()) {
        LogError(sTag, "Could not create sampler");
        return false;
    }

    if (!copyTextureData(img)) {
        LogError(sTag, "Could not upload texture data to device memory");
        return false;
    }

    allocateDescriptorSet();
    updateDescriptorSet();

    return true;
}

void Vk_Texture2D::use() {}

VkDescriptorSet& Vk_Texture2D::getDescriptorSet() {
    return m_descriptorSet;
}

bool Vk_Texture2D::createSampler() {
    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.getVulkanDevice();

    VkBool32 anisotropyEnable = context.getEnabledFeatures().samplerAnisotropy;
    float maxAnisotropy = 16.0F;

    // TODO: This should be configurable from the Renderer instance
    VkSamplerCreateInfo samplerCreateInfo = {
        VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,    // sType
        nullptr,                                  // pNext
        0,                                        // flags
        VK_FILTER_LINEAR,                         // magFilter
        VK_FILTER_LINEAR,                         // minFilter
        VK_SAMPLER_MIPMAP_MODE_NEAREST,           // mipmapMode
        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,    // addressModeU
        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,    // addressModeV
        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,    // addressModeW
        0.0F,                                     // mipLodBias
        anisotropyEnable,                         // anisotropyEnable
        maxAnisotropy,                            // maxAnisotropy
        VK_FALSE,                                 // compareEnable
        VK_COMPARE_OP_ALWAYS,                     // compareOp
        0.0F,                                     // minLod
        0.0F,                                     // maxLod
        VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,  // borderColor
        VK_FALSE                                  // unnormalizedCoordinates
    };

    VkResult result = vkCreateSampler(device, &samplerCreateInfo, nullptr, &m_sampler);

    return result == VK_SUCCESS;
}

bool Vk_Texture2D::copyTextureData(const Image& img) {
    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.getVulkanDevice();
    QueueParameters& graphicsQueue = context.getGraphicsQueue();

    VkResult result = VK_SUCCESS;

    if (!m_stagingBuffer.create(img.getDataSize(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
        LogFatal(sTag, "Could not create Staging Buffer");
    }

    // Prepare data in staging buffer
    void* stagingBufferMemoryPointer;
    result = vkMapMemory(device, m_stagingBuffer.getMemory(), 0, img.getDataSize(), 0, &stagingBufferMemoryPointer);
    if (result != VK_SUCCESS) {
        LogError(sTag, "Could not map memory and upload texture data to a staging buffer");
        return false;
    }

    std::memcpy(stagingBufferMemoryPointer, img.getData(), img.getDataSize());

    VkMappedMemoryRange flushRange = {
        VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,  // sType
        nullptr,                                // pNext
        m_stagingBuffer.getMemory(),            // memory
        0,                                      // offset
        img.getDataSize()                       // size
    };
    vkFlushMappedMemoryRanges(device, 1, &flushRange);

    vkUnmapMemory(device, m_stagingBuffer.getMemory());

    // Prepare command buffer to copy data from staging buffer to a vertex
    // buffer
    VkCommandBufferAllocateInfo allocInfo = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,  // sType
        nullptr,                                         // pNext
        context.getGraphicsQueueCmdPool(),               // commandPool
        VK_COMMAND_BUFFER_LEVEL_PRIMARY,                 // level
        1                                                // commandBufferCount
    };

    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
    result = vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);
    if (result != VK_SUCCESS || commandBuffer == VK_NULL_HANDLE) {
        LogError(sTag, "Could not allocate command buffer");
        return false;
    }

    VkCommandBufferBeginInfo commandBufferBeginInfo = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,  // sType
        nullptr,                                      // pNext
        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,  // flags
        nullptr                                       // pInheritanceInfo
    };

    vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);

    VkImageSubresourceRange imageSubresourceRange = {
        VK_IMAGE_ASPECT_COLOR_BIT,  // aspectMask
        0,                          // baseMipLevel
        1,                          // levelCount
        0,                          // baseArrayLayer
        1                           // layerCount
    };

    VkImageMemoryBarrier imageMemoryBarrierFromUndefinedToTransferDst = {
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,  // sType
        nullptr,                                 // pNext
        0,                                       // srcAccessMask
        VK_ACCESS_TRANSFER_WRITE_BIT,            // dstAccessMask
        VK_IMAGE_LAYOUT_UNDEFINED,               // oldLayout
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,    // newLayout
        VK_QUEUE_FAMILY_IGNORED,                 // srcQueueFamilyIndex
        VK_QUEUE_FAMILY_IGNORED,                 // dstQueueFamilyIndex
        m_image.getHandle(),                     // image
        imageSubresourceRange                    // subresourceRange
    };
    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0,
                         nullptr, 0, nullptr, 1, &imageMemoryBarrierFromUndefinedToTransferDst);

    VkBufferImageCopy bufferImageCopyInfo = {
        0,  // bufferOffset
        0,  // bufferRowLength
        0,  // bufferImageHeight
        {
            // imageSubresource
            VK_IMAGE_ASPECT_COLOR_BIT,  // aspectMask
            0,                          // mipLevel
            0,                          // baseArrayLayer
            1                           // layerCount
        },
        {
            // VkOffset3D imageOffset
            0,  // x
            0,  // y
            0   // z
        },
        {
            // VkExtent3D imageExtent
            img.getSize().x,  // width
            img.getSize().y,  // height
            1                 // depth
        },
    };
    vkCmdCopyBufferToImage(commandBuffer, m_stagingBuffer.getHandle(), m_image.getHandle(),
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferImageCopyInfo);

    VkImageMemoryBarrier imageMemoryBarrierFromTransferToShaderRead = {
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,    // sType
        nullptr,                                   // pNext
        VK_ACCESS_TRANSFER_WRITE_BIT,              // srcAccessMask
        VK_ACCESS_SHADER_READ_BIT,                 // dstAccessMask
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,      // oldLayout
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,  // newLayout
        VK_QUEUE_FAMILY_IGNORED,                   // srcQueueFamilyIndex
        VK_QUEUE_FAMILY_IGNORED,                   // dstQueueFamilyIndex
        m_image.getHandle(),                       // image
        imageSubresourceRange                      // subresourceRange
    };
    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0,
                         nullptr, 0, nullptr, 1, &imageMemoryBarrierFromTransferToShaderRead);

    vkEndCommandBuffer(commandBuffer);

    // Submit command buffer and copy data from staging buffer to a vertex
    // buffer
    VkSubmitInfo submitInfo = {
        VK_STRUCTURE_TYPE_SUBMIT_INFO,  // sType
        nullptr,                        // pNext
        0,                              // waitSemaphoreCount
        nullptr,                        // pWaitSemaphores
        nullptr,                        // pWaitDstStageMask;
        1,                              // commandBufferCount
        &commandBuffer,                 // pCommandBuffers
        0,                              // signalSemaphoreCount
        nullptr                         // pSignalSemaphores
    };

    result = vkQueueSubmit(graphicsQueue.handle, 1, &submitInfo, VK_NULL_HANDLE);

    if (result != VK_SUCCESS) {
        return false;
    }

    vkQueueWaitIdle(graphicsQueue.handle);

    return true;
}

bool Vk_Texture2D::allocateDescriptorSet() {
    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.getVulkanDevice();

    Vk_TextureManager* textureManager = Vk_TextureManager::GetInstancePtr();

    VkResult result = VK_SUCCESS;

    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,  // sType
        nullptr,                                         // pNext
        textureManager->getDescriptorPool(),             // descriptorPool
        1,                                               // descriptorSetCount
        &textureManager->getDescriptorSetLayout()        // pSetLayouts
    };

    result = vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, &m_descriptorSet);
    if (result != VK_SUCCESS) {
        LogError(sTag, "Could not allocate descriptor set");
        return false;
    }

    return true;
}

bool Vk_Texture2D::updateDescriptorSet() {
    // This tell the driver which resources are going to be used by the
    // descriptor set

    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.getVulkanDevice();

    VkDescriptorImageInfo imageInfo = {
        m_sampler,                                // sampler
        m_image.getView(),                        // imageView
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL  // imageLayout
    };

    VkWriteDescriptorSet descriptorWrites = {
        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,     // sType
        nullptr,                                    // pNext
        m_descriptorSet,                            // dstSet
        0,                                          // dstBinding
        0,                                          // dstArrayElement
        1,                                          // descriptorCount
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,  // descriptorType
        &imageInfo,                                 // pImageInfo
        nullptr,                                    // pBufferInfo
        nullptr                                     // pTexelBufferView
    };

    vkUpdateDescriptorSets(device, 1, &descriptorWrites, 0, nullptr);

    return true;
}

}  // namespace engine
