#include <System/LogManager.hpp>
#include <System/StringFormat.hpp>

#include "Vk_Context.hpp"
#include "Vk_Texture2D.hpp"
#include "Vk_TextureManager.hpp"
#include "Vk_Utilities.hpp"

namespace engine {

namespace {

const String sTag("Vk_Texture2D");

}  // namespace

Vk_Texture2D::Vk_Texture2D(Vk_TextureManager* texture_manager)
      : m_texture_manager(texture_manager) {}

Vk_Texture2D::~Vk_Texture2D() {
    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.GetVulkanDevice();
    QueueParameters& graphics_queue = context.GetGraphicsQueue();

    vkQueueWaitIdle(graphics_queue.handle);

    if (m_sampler != VK_NULL_HANDLE) {
        vkDestroySampler(device, m_sampler, nullptr);
        m_sampler = VK_NULL_HANDLE;
    }

    if (m_image.view != VK_NULL_HANDLE) {
        vkDestroyImageView(device, m_image.view, nullptr);
        m_image.view = VK_NULL_HANDLE;
    }

    if (m_image.handle != VK_NULL_HANDLE) {
        vkDestroyImage(device, m_image.handle, nullptr);
        m_image.handle = VK_NULL_HANDLE;
    }

    if (m_memory != VK_NULL_HANDLE) {
        vkFreeMemory(device, m_memory, nullptr);
        m_memory = VK_NULL_HANDLE;
    }
}

bool Vk_Texture2D::LoadFromImage(const Image& img) {
    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.GetVulkanDevice();

    if (!CreateImage(img)) {
        LogError(sTag, "Could not create image");
        return false;
    }

    if (!Vk_Utilities::AllocateMemory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                      m_image.handle, &m_memory)) {
        LogError(sTag, "Could not allocate memory for image");
        return false;
    }

    if (vkBindImageMemory(device, m_image.handle, m_memory, 0) != VK_SUCCESS) {
        LogError(sTag, "Could not bind memory to an image");
        return false;
    }

    if (!CreateImageView()) {
        LogError(sTag, "Could not create image view");
        return false;
    }

    if (!CreateSampler()) {
        LogError(sTag, "Could not create sampler");
        return false;
    }

    if (!CopyTextureData(img)) {
        LogError(sTag, "Could not upload texture data to device memory");
        return false;
    }

    AllocateDescriptorSet();
    UpdateDescriptorSet();

    return true;
}

void Vk_Texture2D::Use() {}

VkDescriptorSet& Vk_Texture2D::GetDescriptorSet() {
    return m_descriptor_set;
}

bool Vk_Texture2D::CreateImage(const Image& img) {
    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.GetVulkanDevice();

    VkResult result = VK_SUCCESS;

    VkImageCreateInfo image_create_info = {
        VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,  // sType;
        nullptr,                              // pNext
        0,                                    // flags
        VK_IMAGE_TYPE_2D,                     // imageType
        VK_FORMAT_R8G8B8A8_UNORM,             // format
        VkExtent3D{
            // extent
            img.GetSize().x,  // width
            img.GetSize().y,  // height
            1                 // depth
        },
        1,                        // mipLevels
        1,                        // arrayLayers
        VK_SAMPLE_COUNT_1_BIT,    // samples
        VK_IMAGE_TILING_OPTIMAL,  // tiling
        (VK_IMAGE_USAGE_TRANSFER_DST_BIT |
         VK_IMAGE_USAGE_SAMPLED_BIT),  // usage
        VK_SHARING_MODE_EXCLUSIVE,     // sharingMode
        0,                             // queueFamilyIndexCount
        nullptr,                       // pQueueFamilyIndices
        VK_IMAGE_LAYOUT_UNDEFINED      // initialLayout
    };

    result =
        vkCreateImage(device, &image_create_info, nullptr, &m_image.handle);

    return result == VK_SUCCESS;
}

bool Vk_Texture2D::CreateImageView() {
    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.GetVulkanDevice();

    VkImageViewCreateInfo image_view_create_info = {
        VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,  // sType
        nullptr,                                   // pNext
        0,                                         // flags
        m_image.handle,                            // image
        VK_IMAGE_VIEW_TYPE_2D,                     // viewType
        VK_FORMAT_R8G8B8A8_UNORM,                  // format
        {
            // components
            VK_COMPONENT_SWIZZLE_IDENTITY,  // r
            VK_COMPONENT_SWIZZLE_IDENTITY,  // g
            VK_COMPONENT_SWIZZLE_IDENTITY,  // b
            VK_COMPONENT_SWIZZLE_IDENTITY   // a
        },
        {
            // subresourceRange
            VK_IMAGE_ASPECT_COLOR_BIT,  // aspectMask
            0,                          // baseMipLevel
            1,                          // levelCount
            0,                          // baseArrayLayer
            1                           // layerCount
        },
    };

    return vkCreateImageView(device, &image_view_create_info, nullptr,
                             &m_image.view) == VK_SUCCESS;
}

bool Vk_Texture2D::CreateSampler() {
    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.GetVulkanDevice();

    VkResult result = VK_SUCCESS;

    // TODO: This should be configurable from the Renderer instance
    VkSamplerCreateInfo sampler_create_info = {
        VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,    // sType
        nullptr,                                  // pNext
        0,                                        // flags
        VK_FILTER_LINEAR,                         // magFilter
        VK_FILTER_LINEAR,                         // minFilter
        VK_SAMPLER_MIPMAP_MODE_NEAREST,           // mipmapMode
        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,    // addressModeU
        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,    // addressModeV
        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,    // addressModeW
        0.0f,                                     // mipLodBias
        VK_FALSE,                                 // anisotropyEnable
        1.0f,                                     // maxAnisotropy
        VK_FALSE,                                 // compareEnable
        VK_COMPARE_OP_ALWAYS,                     // compareOp
        0.0f,                                     // minLod
        0.0f,                                     // maxLod
        VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,  // borderColor
        VK_FALSE                                  // unnormalizedCoordinates
    };

    result = vkCreateSampler(device, &sampler_create_info, nullptr, &m_sampler);

    return result == VK_SUCCESS;
}

bool Vk_Texture2D::CopyTextureData(const Image& img) {
    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.GetVulkanDevice();
    QueueParameters& graphics_queue = context.GetGraphicsQueue();

    VkResult result = VK_SUCCESS;

    if (!m_staging_buffer.Create(img.GetDataSize(),
                                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
        LogFatal(sTag, "Could not create Staging Buffer");
    }

    // Prepare data in staging buffer
    void* staging_buffer_memory_pointer;
    result = vkMapMemory(device, m_staging_buffer.GetMemory(), 0,
                         img.GetDataSize(), 0, &staging_buffer_memory_pointer);
    if (result != VK_SUCCESS) {
        LogError(sTag,
                 "Could not map memory and upload "
                 "texture data to a staging buffer");
        return false;
    }

    std::memcpy(staging_buffer_memory_pointer, img.GetData(),
                img.GetDataSize());

    VkMappedMemoryRange flush_range = {
        VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,  // sType
        nullptr,                                // pNext
        m_staging_buffer.GetMemory(),           // memory
        0,                                      // offset
        img.GetDataSize()                       // size
    };
    vkFlushMappedMemoryRanges(device, 1, &flush_range);

    vkUnmapMemory(device, m_staging_buffer.GetMemory());

    // Prepare command buffer to copy data from staging buffer to a vertex
    // buffer
    VkCommandBufferAllocateInfo allocInfo = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,  // sType
        nullptr,                                         // pNext
        context.GetGraphicsQueueCmdPool(),               // commandPool
        VK_COMMAND_BUFFER_LEVEL_PRIMARY,                 // level
        1                                                // commandBufferCount
    };

    VkCommandBuffer command_buffer = VK_NULL_HANDLE;
    result = vkAllocateCommandBuffers(device, &allocInfo, &command_buffer);
    if (result != VK_SUCCESS || command_buffer == VK_NULL_HANDLE) {
        LogError(sTag, "Could not allocate command buffer");
        return false;
    }

    VkCommandBufferBeginInfo command_buffer_begin_info = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,  // sType
        nullptr,                                      // pNext
        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,  // flags
        nullptr                                       // pInheritanceInfo
    };

    vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info);

    VkImageSubresourceRange image_subresource_range = {
        VK_IMAGE_ASPECT_COLOR_BIT,  // aspectMask
        0,                          // baseMipLevel
        1,                          // levelCount
        0,                          // baseArrayLayer
        1                           // layerCount
    };

    VkImageMemoryBarrier image_memory_barrier_from_undefined_to_transfer_dst = {
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,  // sType
        nullptr,                                 // pNext
        0,                                       // srcAccessMask
        VK_ACCESS_TRANSFER_WRITE_BIT,            // dstAccessMask
        VK_IMAGE_LAYOUT_UNDEFINED,               // oldLayout
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,    // newLayout
        VK_QUEUE_FAMILY_IGNORED,                 // srcQueueFamilyIndex
        VK_QUEUE_FAMILY_IGNORED,                 // dstQueueFamilyIndex
        m_image.GetHandle(),                     // image
        image_subresource_range                  // subresourceRange
    };
    vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                         VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0,
                         nullptr, 1,
                         &image_memory_barrier_from_undefined_to_transfer_dst);

    VkBufferImageCopy buffer_image_copy_info = {
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
            img.GetSize().x,  // width
            img.GetSize().y,  // height
            1                 // depth
        },
    };
    vkCmdCopyBufferToImage(command_buffer, m_staging_buffer.GetHandle(),
                           m_image.handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           1, &buffer_image_copy_info);

    VkImageMemoryBarrier image_memory_barrier_from_transfer_to_shader_read = {
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,    // sType
        nullptr,                                   // pNext
        VK_ACCESS_TRANSFER_WRITE_BIT,              // srcAccessMask
        VK_ACCESS_SHADER_READ_BIT,                 // dstAccessMask
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,      // oldLayout
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,  // newLayout
        VK_QUEUE_FAMILY_IGNORED,                   // srcQueueFamilyIndex
        VK_QUEUE_FAMILY_IGNORED,                   // dstQueueFamilyIndex
        m_image.GetHandle(),                       // image
        image_subresource_range                    // subresourceRange
    };
    vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr,
                         0, nullptr, 1,
                         &image_memory_barrier_from_transfer_to_shader_read);

    vkEndCommandBuffer(command_buffer);

    // Submit command buffer and copy data from staging buffer to a vertex
    // buffer
    VkSubmitInfo submit_info = {
        VK_STRUCTURE_TYPE_SUBMIT_INFO,  // sType
        nullptr,                        // pNext
        0,                              // waitSemaphoreCount
        nullptr,                        // pWaitSemaphores
        nullptr,                        // pWaitDstStageMask;
        1,                              // commandBufferCount
        &command_buffer,                // pCommandBuffers
        0,                              // signalSemaphoreCount
        nullptr                         // pSignalSemaphores
    };

    result =
        vkQueueSubmit(graphics_queue.handle, 1, &submit_info, VK_NULL_HANDLE);

    if (result != VK_SUCCESS) {
        return false;
    }

    vkQueueWaitIdle(graphics_queue.handle);

    return true;
}

bool Vk_Texture2D::AllocateDescriptorSet() {
    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.GetVulkanDevice();

    VkResult result = VK_SUCCESS;

    VkDescriptorSetAllocateInfo descriptor_set_allocate_info = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,  // sType
        nullptr,                                         // pNext
        m_texture_manager->GetDescriptorPool(),          // descriptorPool
        1,                                               // descriptorSetCount
        &m_texture_manager->GetDescriptorSetLayout()     // pSetLayouts
    };

    result = vkAllocateDescriptorSets(device, &descriptor_set_allocate_info,
                                      &m_descriptor_set);
    if (result != VK_SUCCESS) {
        LogError(sTag, "Could not allocate descriptor set");
        return false;
    }

    return true;
}

bool Vk_Texture2D::UpdateDescriptorSet() {
    // This tell the driver which resources are going to be used by the
    // descriptor set

    Vk_Context& context = Vk_Context::GetInstance();
    VkDevice& device = context.GetVulkanDevice();

    VkDescriptorImageInfo image_info = {
        m_sampler,                                // sampler
        m_image.view,                             // imageView
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL  // imageLayout
    };

    VkWriteDescriptorSet descriptor_writes = {
        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,     // sType
        nullptr,                                    // pNext
        m_descriptor_set,                           // dstSet
        0,                                          // dstBinding
        0,                                          // dstArrayElement
        1,                                          // descriptorCount
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,  // descriptorType
        &image_info,                                // pImageInfo
        nullptr,                                    // pBufferInfo
        nullptr                                     // pTexelBufferView
    };

    vkUpdateDescriptorSets(device, 1, &descriptor_writes, 0, nullptr);

    return true;
}

}  // namespace engine
