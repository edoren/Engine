#pragma once

#include <Math/Math.hpp>
#include <Renderer/Shader.hpp>
#include <System/String.hpp>

#include "Vk_Buffer.hpp"
#include "Vk_Config.hpp"
#include "Vk_Dependencies.hpp"
#include "Vk_VertexLayout.hpp"

namespace engine {

class VULKAN_PLUGIN_API Vk_Shader : public Shader {
public:
    Vk_Shader();
    Vk_Shader(Vk_Shader&& other);
    ~Vk_Shader() override;

    Vk_Shader& operator=(Vk_Shader&& other);

    bool loadFromMemory(const byte* source, std::size_t source_size, ShaderType type) override;

    UniformBufferObject& getUbo();
    UniformBufferObject& getUboDynamic();

    const Vk_VertexLayout& getVertexLayout() const;

    VkShaderModule& getModule(ShaderType type);

    VkDescriptorSet& getUboDescriptorSet();

    VkDescriptorSetLayout& getUboDescriptorSetLayout();

    bool uploadUniformBuffers();

protected:
    void setDescriptor(json&& descriptor) override;

private:
    bool createUboDescriptorSetLayout();
    bool allocateUboDescriptorSet();
    bool updateUboDescriptorSet();

    bool createUniformBuffers();

    json m_descriptor;

    UniformBufferObject m_ubo;
    UniformBufferObject m_ubo_dynamic;

    Vk_VertexLayout m_vertex_layout;

    std::array<VkShaderModule, sShaderTypeCount> m_modules;

    struct {
        Vk_Buffer _static;
        Vk_Buffer _dynamic;
    } m_uniform_buffers;

    VkDescriptorSetLayout m_ubo_descriptor_set_layout;
    VkDescriptorSet m_ubo_descriptor_set;
};

}  // namespace engine
