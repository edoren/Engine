#pragma once

#include "Vk_Config.hpp"
#include "Vk_VulkanParameters.hpp"
#include "Vk_Dependencies.hpp"

namespace engine {

class VULKAN_PLUGIN_API Vk_Context : public Singleton<Vk_Context> {
public:
    Vk_Context();

    ~Vk_Context();

    bool Initialize();

    void Shutdown();

    VkInstance& GetVulkanInstance();
    VkDevice& GetVulkanDevice();

    PhysicalDeviceParameters& GetPhysicalDevice();

    QueueParameters& GetGraphicsQueue();

    // Override standard Singleton retrieval.
    //     @remarks
    //         Why do we do this? Well, it's because the Singleton
    //         implementation is in a .h file, which means it gets compiled
    //         into anybody who includes it. This is needed for the
    //         Singleton template to work, but we actually only want it
    //         compiled into the implementation of the class based on the
    //         Singleton, not all of them. If we don't change this, we get
    //         link errors when trying to use the Singleton-based class from
    //         an outside dll.
    //     @par
    //         This method just delegates to the template version anyway,
    //         but the implementation stays in this single compilation unit,
    //         preventing link errors.
    static Vk_Context& GetInstance();

    // Override standard Singleton retrieval.
    //     @remarks
    //         Why do we do this? Well, it's because the Singleton
    //         implementation is in a .h file, which means it gets compiled
    //         into anybody who includes it. This is needed for the
    //         Singleton template to work, but we actually only want it
    //         compiled into the implementation of the class based on the
    //         Singleton, not all of them. If we don't change this, we get
    //         link errors when trying to use the Singleton-based class from
    //         an outside dll.
    //     @par
    //         This method just delegates to the template version anyway,
    //         but the implementation stays in this single compilation unit,
    //         preventing link errors.
    static Vk_Context* GetInstancePtr();

private:
    bool CreateInstance();
    bool CreateDevice();

    bool SelectPhysicalDevice(VkPhysicalDevice& physical_device);

    bool CheckValidationLayerSupport() const;
    bool CheckInstanceExtensionsSupport() const;

private:
    VkInstance m_instance;
    VkDevice m_device;
    PhysicalDeviceParameters m_physical_device;

    QueueParameters m_graphics_queue;

    bool m_validation_layers_enabled;
    std::vector<const char*> m_validation_layers;

    std::vector<const char*> m_instance_extensions;
    std::vector<const char*> m_device_extensions;
};

}  // namespace engine
