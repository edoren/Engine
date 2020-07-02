#pragma once

#include <Util/Container/Vector.hpp>
#include <Util/Singleton.hpp>

#include "Vk_Config.hpp"
#include "Vk_Dependencies.hpp"
#include "Vk_VulkanParameters.hpp"

namespace engine {

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

    /**
     * @brief Override standard Singleton retrieval.
     *
     * @remarks Why do we do this? Well, it's because the Singleton
     *          implementation is in a .hpp file, which means it gets
     *          compiled into anybody who includes it. This is needed
     *          for the Singleton template to work, but we actually
     *          only compiled into the implementation of the class
     *          based on the Singleton, not all of them. If we don't
     *          change this, we get link errors when trying to use the
     *          Singleton-based class from an outside dll.
     *
     * @par This method just delegates to the template version anyway,
     *      but the implementation stays in this single compilation unit,
     *      preventing link errors.
     */
    static Vk_Context& GetInstance();

    /**
     * @brief Override standard Singleton retrieval.
     *
     * @remarks Why do we do this? Well, it's because the Singleton
     *          implementation is in a .hpp file, which means it gets
     *          compiled into anybody who includes it. This is needed
     *          for the Singleton template to work, but we actually
     *          only compiled into the implementation of the class
     *          based on the Singleton, not all of them. If we don't
     *          change this, we get link errors when trying to use the
     *          Singleton-based class from an outside dll.
     *
     * @par This method just delegates to the template version anyway,
     *      but the implementation stays in this single compilation unit,
     *      preventing link errors.
     */
    static Vk_Context* GetInstancePtr();

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

}  // namespace engine
