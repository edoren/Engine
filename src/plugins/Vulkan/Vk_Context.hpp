#pragma once

#include <Util/Container/Vector.hpp>
#include <Util/Singleton.hpp>

#include "Vk_Config.hpp"
#include "Vk_Dependencies.hpp"
#include "Vk_VulkanParameters.hpp"

namespace engine::plugin::vulkan {

class VULKAN_PLUGIN_API Vk_Context : public Singleton<Vk_Context> {
public:
    Vk_Context();

    ~Vk_Context();

    bool initialize();

    void shutdown();

    VkInstance& getVulkanInstance();
    VkDevice& getVulkanDevice();

    PhysicalDeviceParameters& getPhysicalDevice();

    QueueParameters& getGraphicsQueue();

    VkCommandPool& getGraphicsQueueCmdPool();

    VkDescriptorPool& getUboDescriptorPool();

    const VkPhysicalDeviceFeatures& getEnabledFeatures();

private:
    bool createInstance();
    bool createDevice();

    bool selectPhysicalDevice(VkPhysicalDevice& physicalDevice);

    bool checkValidationLayerSupport() const;
    bool checkInstanceExtensionsSupport() const;

    bool createVulkanCommandPool(QueueParameters& queue, VkCommandPool* cmdPool);

    bool createUboDescriptorPool();

    VkInstance m_instance;
    VkDevice m_device;
    PhysicalDeviceParameters m_physicalDevice;

    VkPhysicalDeviceFeatures m_enabledFeatures;

    QueueParameters m_graphicsQueue;

    VkCommandPool m_graphicsQueueCmdPool;

    VkDescriptorPool m_uboDescriptorPool;

    VkDebugReportCallbackEXT m_debugReportCallback;

    bool m_validationLayersEnabled;
    Vector<const char*> m_validationLayers;

    Vector<const char*> m_instanceExtensions;
    Vector<const char*> m_deviceExtensions;
};

}  // namespace engine::plugin::vulkan
