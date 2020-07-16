#include <System/LogManager.hpp>
#include <System/StringFormat.hpp>
#include <System/StringView.hpp>
#include <Util/Container/Vector.hpp>
#include <Util/Function.hpp>

#include "Vk_Context.hpp"

#include <array>

namespace engine::plugin::vulkan {

namespace {

const StringView sTag("Vk_Context");
const StringView sTagVkDebug("Vk_ValidationLayers");

const uint32 sMaxUBODescriptorSets(10);
const uint32 sMaxUBODynamicDescriptorSets(10);

VKAPI_ATTR VkBool32 VKAPI_CALL DebugReportCallback(VkDebugReportFlagsEXT msgFlags,
                                                   VkDebugReportObjectTypeEXT objType,
                                                   uint64_t srcObject,
                                                   size_t location,
                                                   int32_t msgCode,
                                                   const char* pLayerPrefix,
                                                   const char* pMsg,
                                                   void* pUserData) {
    ENGINE_UNUSED(objType);
    ENGINE_UNUSED(srcObject);
    ENGINE_UNUSED(location);
    ENGINE_UNUSED(pLayerPrefix);
    ENGINE_UNUSED(pUserData);

    String outputMsg("{} - Code: {}"_format(pMsg, msgCode));

    if (msgFlags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
        LogError(sTagVkDebug, outputMsg);
    } else if (msgFlags & VK_DEBUG_REPORT_WARNING_BIT_EXT || msgFlags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) {
        LogWarning(sTagVkDebug, outputMsg);
    } else if (msgFlags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) {
        LogInfo(sTagVkDebug, outputMsg);
    } else if (msgFlags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) {
        LogDebug(sTagVkDebug, outputMsg);
    }

    // Returning false tells the layer not to stop when the event occurs, so
    // they see the same behavior with and without validation layers enabled.
    return VK_FALSE;
}

bool CheckExtensionAvailability(const char* str, const Vector<VkExtensionProperties>& vec) {
    for (const auto& property : vec) {
        if (!strcmp(str, property.extensionName)) {
            return true;
        }
    }
    return false;
}

bool CheckLayerAvailability(const char* str, const Vector<VkLayerProperties>& vec) {
    for (const auto& property : vec) {
        if (!strcmp(str, property.layerName)) {
            return true;
        }
    }
    return false;
}

}  // namespace

Vk_Context::Vk_Context()
      : m_instance(VK_NULL_HANDLE),
        m_device(VK_NULL_HANDLE),
        m_graphicsQueueCmdPool(VK_NULL_HANDLE),
        m_uboDescriptorPool(VK_NULL_HANDLE),
        m_debugReportCallback(VK_NULL_HANDLE),
#ifdef ENGINE_DEBUG
        m_validationLayersEnabled(true) {
#else
        m_validationLayersEnabled(false){
#endif

}  // namespace engine::plugin::vulkan

Vk_Context::~Vk_Context() {
    shutdown();
}

bool Vk_Context::initialize() {
    // Add the required validation layers
    if (m_validationLayersEnabled) {
#if PLATFORM_IS(PLATFORM_ANDROID)
        m_validationLayers = {"VK_LAYER_GOOGLE_threading", "VK_LAYER_LUNARG_parameter_validation",
                              "VK_LAYER_LUNARG_object_tracker", "VK_LAYER_LUNARG_core_validation",
                              "VK_LAYER_GOOGLE_unique_objects"};
#elif PLATFORM_IS(PLATFORM_IOS)
        m_validationLayersEnabled = false;
#else
        m_validationLayers = {"VK_LAYER_KHRONOS_validation"};
#endif
    }

    // Add the required Instance extensions
    m_instanceExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
#if defined(SDL_VIDEO_DRIVER_WINDOWS)
    m_instanceExtensions.push_back("VK_KHR_win32_surface");
#endif
#if defined(SDL_VIDEO_DRIVER_X11)
    m_instanceExtensions.push_back("VK_KHR_xcb_surface");
    m_instanceExtensions.push_back("VK_KHR_xlib_surface");
#endif
#if defined(SDL_VIDEO_DRIVER_WAYLAND)
    m_instanceExtensions.push_back("VK_KHR_wayland_surface");
#endif
#if defined(SDL_VIDEO_DRIVER_MIR)
    m_instanceExtensions.push_back("VK_KHR_mir_surface");
#endif
#if defined(SDL_VIDEO_DRIVER_ANDROID)
    m_instanceExtensions.push_back("VK_KHR_android_surface");
#endif
#if defined(SDL_VIDEO_DRIVER_COCOA)
    m_instanceExtensions.push_back("VK_MVK_macos_surface");
#endif
#if defined(SDL_VIDEO_DRIVER_UIKIT)
    m_instanceExtensions.push_back("VK_MVK_ios_surface");
#endif
    if (m_instanceExtensions.size() < 2) {
        LogFatal(sTag, "Platform does not has a supported surface extension");
    }
    if (m_validationLayersEnabled) {
        m_instanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    }

    // Add the required Device extensions
    m_deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    // Check the validation layers support
    if (m_validationLayersEnabled && !checkValidationLayerSupport()) {
        LogFatal(sTag, "Validation layers requested, but not available");
        return false;
    }

    createInstance();
    createDevice();

    if (m_validationLayersEnabled && !m_debugReportCallback) {
        PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT;

        vkCreateDebugReportCallbackEXT = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(
            vkGetInstanceProcAddr(m_instance, "vkCreateDebugReportCallbackEXT"));

        // Create the debug callback with desired settings
        if (vkCreateDebugReportCallbackEXT) {
            VkDebugReportCallbackCreateInfoEXT createInfo = {
                .sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT,
                .pNext = nullptr,
                .flags = (VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT |
                          VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT),
                .pfnCallback = DebugReportCallback,
                .pUserData = nullptr,
            };

            vkCreateDebugReportCallbackEXT(m_instance, &createInfo, nullptr, &m_debugReportCallback);
        }
    }

    if (!createVulkanCommandPool(m_graphicsQueue, &m_graphicsQueueCmdPool)) {
        return false;
    }

    if (!createUboDescriptorPool()) {
        LogError(sTag, "Could not create the UBO descriptor pool");
        return false;
    }

    return true;
}

void Vk_Context::shutdown() {
    if (m_device) {
        if (m_graphicsQueueCmdPool) {
            vkDestroyCommandPool(m_device, m_graphicsQueueCmdPool, nullptr);
            m_graphicsQueueCmdPool = VK_NULL_HANDLE;
        }

        if (m_uboDescriptorPool) {
            vkDestroyDescriptorPool(m_device, m_uboDescriptorPool, nullptr);
            m_uboDescriptorPool = VK_NULL_HANDLE;
        }

        vkDestroyDevice(m_device, nullptr);
        m_device = VK_NULL_HANDLE;
    }

    if (m_validationLayersEnabled && m_debugReportCallback) {
        PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT;
        vkDestroyDebugReportCallbackEXT = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(
            vkGetInstanceProcAddr(m_instance, "vkDestroyDebugReportCallbackEXT"));

        if (vkDestroyDebugReportCallbackEXT) {
            vkDestroyDebugReportCallbackEXT(m_instance, m_debugReportCallback, nullptr);
        }
    }

    if (m_instance) {
        vkDestroyInstance(m_instance, nullptr);
        m_instance = VK_NULL_HANDLE;
    }
}

VkInstance& Vk_Context::getVulkanInstance() {
    return m_instance;
}

VkDevice& Vk_Context::getVulkanDevice() {
    return m_device;
}

PhysicalDeviceParameters& Vk_Context::getPhysicalDevice() {
    return m_physicalDevice;
}

QueueParameters& Vk_Context::getGraphicsQueue() {
    return m_graphicsQueue;
}

VkCommandPool& Vk_Context::getGraphicsQueueCmdPool() {
    return m_graphicsQueueCmdPool;
}

VkDescriptorPool& Vk_Context::getUboDescriptorPool() {
    return m_uboDescriptorPool;
}

const VkPhysicalDeviceFeatures& Vk_Context::getEnabledFeatures() {
    return m_enabledFeatures;
}

bool Vk_Context::createInstance() {
    // Define the application information
    VkApplicationInfo appInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = nullptr,
        .pApplicationName = "Engine",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "Engine",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_MAKE_VERSION(1, 0, 0),
    };

    // Check that all the instance extensions are supported
    if (!checkInstanceExtensionsSupport()) {
        LogFatal(sTag, "Error instance extensions");
        return false;
    };

    // Define all the information for the instance
    VkInstanceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = VkInstanceCreateFlags(),
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = static_cast<uint32>(m_validationLayers.size()),
        .ppEnabledLayerNames = m_validationLayers.data(),
        .enabledExtensionCount = static_cast<uint32>(m_instanceExtensions.size()),
        .ppEnabledExtensionNames = m_instanceExtensions.data(),
    };

    // Create the Vulkan instance based on the provided info
    VkResult result = vkCreateInstance(&createInfo, nullptr, &m_instance);
    if (result != VK_SUCCESS) {
        LogFatal(sTag, "Failed to create Vulkan instance");
        return false;
    }

    return true;
}

bool Vk_Context::createDevice() {
    VkResult result = VK_SUCCESS;

    // Query all the avaliable physical devices
    uint32 physicalDevicesCount = 0;
    Vector<VkPhysicalDevice> physicalDevices;

    result = vkEnumeratePhysicalDevices(m_instance, &physicalDevicesCount, nullptr);
    if (physicalDevicesCount > 0 && result == VK_SUCCESS) {
        physicalDevices.resize(physicalDevicesCount);
        result = vkEnumeratePhysicalDevices(m_instance, &physicalDevicesCount, physicalDevices.data());
    }
    if (physicalDevicesCount == 0 || result != VK_SUCCESS) {
        LogFatal(sTag, "Error querying physical devices");
        return false;
    }

    // Check all the queried physical devices for one with the required
    // caracteristics and avaliable queues
    for (auto& physicalDevice : physicalDevices) {
        if (selectPhysicalDevice(physicalDevice)) {
            break;
        }
    }
    if (!m_physicalDevice.handle || m_graphicsQueue.familyIndex == UINT32_MAX) {
        LogFatal("Vk_Core", "No physical device that supports the required caracteristics");
        return false;
    }

    // Define the queue families information
    Vector<float> queuePriorities = {1.0F};
    Vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    queueCreateInfos.push_back({
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .pNext = nullptr,
        .flags = VkDeviceQueueCreateFlags(),
        .queueFamilyIndex = m_graphicsQueue.familyIndex,
        .queueCount = static_cast<uint32>(queuePriorities.size()),
        .pQueuePriorities = queuePriorities.data(),
    });

    // TODO: Configure this in runtime
    m_enabledFeatures = {};
    m_enabledFeatures.samplerAnisotropy = m_physicalDevice.features.samplerAnisotropy;

    // Define all the information for the logical device
    VkDeviceCreateInfo deviceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = nullptr,
        .flags = VkDeviceCreateFlags(),
        .queueCreateInfoCount = static_cast<uint32>(queueCreateInfos.size()),
        .pQueueCreateInfos = queueCreateInfos.data(),
        .enabledLayerCount = static_cast<uint32>(m_validationLayers.size()),
        .ppEnabledLayerNames = m_validationLayers.data(),
        .enabledExtensionCount = static_cast<uint32>(m_deviceExtensions.size()),
        .ppEnabledExtensionNames = m_deviceExtensions.data(),
        .pEnabledFeatures = &m_enabledFeatures,
    };

    // Create the logical device based on the retrived info
    result = vkCreateDevice(m_physicalDevice.handle, &deviceCreateInfo, nullptr, &m_device);
    if (result != VK_SUCCESS) {
        LogFatal(sTag, "Could not create Vulkan device");
        return false;
    }

    // Get the Queue handles
    vkGetDeviceQueue(m_device, m_graphicsQueue.familyIndex, 0, &m_graphicsQueue.handle);

    return true;
}

bool Vk_Context::selectPhysicalDevice(VkPhysicalDevice& physicalDevice) {
    VkResult result = VK_SUCCESS;

    // Check the PhysicalDevice properties and features
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);
    vkGetPhysicalDeviceFeatures(physicalDevice, &features);

    uint32 majorVersion = VK_VERSION_MAJOR(properties.apiVersion);
    // uint32 minor_version = VK_VERSION_MINOR(properties.apiVersion);
    // uint32 patch_version = VK_VERSION_PATCH(properties.apiVersion);

    if (majorVersion < 1 && properties.limits.maxImageDimension2D < 4096) {
        LogError(sTag, "Physical device {} doesn't support required parameters", properties.deviceName);
        return false;
    }

    // Check if the physical device support the required extensions
    uint32 extensionsCount = 0;
    Vector<VkExtensionProperties> availableExtensions;

    result = vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionsCount, nullptr);
    if (result == VK_SUCCESS && extensionsCount > 0) {
        availableExtensions.resize(extensionsCount);
        result =
            vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionsCount, &availableExtensions[0]);
    }

    // Check that the avaliable extensions could be retreived
    if (result != VK_SUCCESS || extensionsCount == 0) {
        LogError(sTag, "Error occurred during physical device {} extensions enumeration", properties.deviceName);
        return false;
    }

    // Check that all the required device extensions exists
    for (auto& mDeviceExtension : m_deviceExtensions) {
        if (!CheckExtensionAvailability(mDeviceExtension, availableExtensions)) {
            LogError(sTag, "Physical device {} doesn't support extension named \"{}\"", properties.deviceName,
                     mDeviceExtension);
            return false;
        }
    }

    // Retreive all the queue families properties
    uint32 queueFamiliesCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamiliesCount, nullptr);
    if (queueFamiliesCount == 0) {
        LogError(sTag, "Physical device {} doesn't have any queue families", properties.deviceName);
        return false;
    }
    Vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamiliesCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamiliesCount, queueFamilyProperties.data());

    // Find a queue family that supports graphics queue
    uint32 graphicsQueueFamilyIndex = UINT32_MAX;
    for (uint32 i = 0; i < queueFamiliesCount; i++) {
        // Select first queue that supports graphics
        if (queueFamilyProperties[i].queueCount > 0 && queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            graphicsQueueFamilyIndex = i;
        }
    }

    // If this device doesn't support queues with graphics and present
    // capabilities don't use it
    if (graphicsQueueFamilyIndex == UINT32_MAX) {
        LogError(sTag, "Could not find queue family with required properties on physical device: {}",
                 properties.deviceName);
        return false;
    }

    // Set the PhysicalDeviceProperties
    m_physicalDevice.handle = physicalDevice;
    m_physicalDevice.properties = properties;
    m_physicalDevice.features = features;

    // Set the graphical QueueProperties
    m_graphicsQueue.familyIndex = graphicsQueueFamilyIndex;
    m_graphicsQueue.properties = queueFamilyProperties[m_graphicsQueue.familyIndex];

    return true;
}

bool Vk_Context::checkValidationLayerSupport() const {
    VkResult result = VK_SUCCESS;

    // Get the avaliable layers
    uint32 layerCount = 0;
    Vector<VkLayerProperties> avaliableLayers;
    result = vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    if (layerCount > 0 && result == VK_SUCCESS) {
        avaliableLayers.resize(layerCount);
        result = vkEnumerateInstanceLayerProperties(&layerCount, avaliableLayers.data());
    }

    // Check that the avaliable layers could be retreived
    if (layerCount == 0 || result != VK_SUCCESS) {
        LogError(sTag, "Error occurred during validation layers enumeration");
        return false;
    }

    Vector<const char*> availableLayerList = avaliableLayers.map([](const auto& layer) { return layer.layerName; });
    LogInfo(sTag, "Available Layers: {}", availableLayerList);

    // Check that all the validation layers exists
    for (const auto& requestedLayer : m_validationLayers) {
        if (!CheckLayerAvailability(requestedLayer, avaliableLayers)) {
            LogError(sTag, "Could not find validation layer named: {}", requestedLayer);
            return false;
        }
    }

    return true;
}

bool Vk_Context::checkInstanceExtensionsSupport() const {
    VkResult result = VK_SUCCESS;

    // Get the avaliable extensions
    uint32 extensionsCount = 0;
    Vector<VkExtensionProperties> availableExtensions;
    result = vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount, nullptr);
    if (extensionsCount > 0 && result == VK_SUCCESS) {
        availableExtensions.resize(extensionsCount);
        result = vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount, availableExtensions.data());
    }

    // Check that the avaliable extensions could be retreived
    if (extensionsCount == 0 || result != VK_SUCCESS) {
        LogError(sTag, "Error occurred during instance extensions enumeration");
        return false;
    }

#if defined(SDL_VIDEO_DRIVER_X11)
    const char* x11_extension_to_ignore = nullptr;
    if (CheckExtensionAvailability("VK_KHR_xlib_surface", available_extensions)) {
        x11_extension_to_ignore = "VK_KHR_xcb_surface";
    } else if (CheckExtensionAvailability("VK_KHR_xcb_surface", available_extensions)) {
        x11_extension_to_ignore = "VK_KHR_xlib_surface";
    } else {
        LogError(sTag,
                 "Neither the 'VK_KHR_xlib_surface' extension nor the 'VK_KHR_xcb_surface' extension are supported by "
                 "the current system.");
        return false;
    }
#endif

    // Check that all the required instance extensions exists
    bool allExtensionsFound = true;
    for (const char* instanceExtension : m_instanceExtensions) {
#if defined(SDL_VIDEO_DRIVER_X11)
        if (!strcmp(x11_extension_to_ignore, instance_extension)) {
            continue;
        }
#endif
        if (!CheckExtensionAvailability(instanceExtension, availableExtensions)) {
            LogError(sTag, "Could not find instance extension named: {}", instanceExtension);
            allExtensionsFound = false;
        }
    }

    return allExtensionsFound;
}

bool Vk_Context::createVulkanCommandPool(QueueParameters& queue, VkCommandPool* cmdPool) {
    VkResult result = VK_SUCCESS;

    // Create the pool for the command buffers
    VkCommandPoolCreateInfo cmdPoolCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = (VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT),
        .queueFamilyIndex = queue.familyIndex,
    };

    result = vkCreateCommandPool(m_device, &cmdPoolCreateInfo, nullptr, cmdPool);
    if (result != VK_SUCCESS) {
        LogError(sTag, "Could not create a command pool");
        return false;
    }

    return true;
}

bool Vk_Context::createUboDescriptorPool() {
    VkResult result = VK_SUCCESS;

    std::array<VkDescriptorPoolSize, 2> poolSizes = {{
        {.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = sMaxUBODescriptorSets},
        {.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, .descriptorCount = sMaxUBODynamicDescriptorSets},
    }};

    uint32 maxSets = 0;
    for (VkDescriptorPoolSize& poolSize : poolSizes) {
        maxSets += poolSize.descriptorCount;
    }

    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .maxSets = maxSets,
        .poolSizeCount = static_cast<uint32>(poolSizes.size()),
        .pPoolSizes = poolSizes.data(),
    };

    result = vkCreateDescriptorPool(m_device, &descriptorPoolCreateInfo, nullptr, &m_uboDescriptorPool);
    if (result != VK_SUCCESS) {
        LogError(sTag, "Could not create descriptor pool");
        return false;
    }

    return true;
}

}  // namespace engine::plugin::vulkan
