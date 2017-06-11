#pragma once

#include "Vk_Config.hpp"
#include "Vk_VulkanParameters.hpp"
#include "Vk_Dependencies.hpp"

namespace engine {

class VULKAN_PLUGIN_API Vk_Core : NonCopyable {
public:
    Vk_Core();

    ~Vk_Core();

    bool Initialize();

    void Shutdown();

    vk::Instance& GetInstance();

    vk::Device& GetDevice();

    PhysicalDeviceParameters& GetPhysicalDevice();

    QueueParameters& GetGraphicsQueue();

private:
    bool CreateInstance();
    bool CreateDevice();

    bool SelectPhysicalDevice(vk::PhysicalDevice& physical_device);

    bool CheckValidationLayerSupport() const;
    bool CheckInstanceExtensionsSupport() const;

private:
    vk::Instance m_instance;
    vk::Device m_device;
    PhysicalDeviceParameters m_physical_device;

    QueueParameters m_graphics_queue;

    bool m_validation_layers_enabled;
    std::vector<const char*> m_validation_layers;

    std::vector<const char*> m_instance_extensions;
    std::vector<const char*> m_device_extensions;
};

}  // namespace engine
