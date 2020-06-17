#include <System/LogManager.hpp>
#include <System/StringFormat.hpp>
#include <Util/Container/Vector.hpp>
#include <Util/Function.hpp>

#include "Vk_Context.hpp"

namespace engine {

namespace {

const String sTag("Vk_Context");
const String sTagVkDebug("Vk_ValidationLayers");

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

    String output_msg("{} - Code: {}"_format(pMsg, msgCode));

    if (msgFlags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
        LogError(sTagVkDebug, output_msg);
    } else if (msgFlags & VK_DEBUG_REPORT_WARNING_BIT_EXT || msgFlags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) {
        LogWarning(sTagVkDebug, output_msg);
    } else if (msgFlags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) {
        LogInfo(sTagVkDebug, output_msg);
    } else if (msgFlags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) {
        LogDebug(sTagVkDebug, output_msg);
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

template <>
Vk_Context* Singleton<Vk_Context>::sInstance = nullptr;

Vk_Context& Vk_Context::GetInstance() {
    return Singleton<Vk_Context>::GetInstance();
}

Vk_Context* Vk_Context::GetInstancePtr() {
    return Singleton<Vk_Context>::GetInstancePtr();
}

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

}  // namespace engine

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
            VkDebugReportCallbackCreateInfoEXT create_info = {
                VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT,  // sType
                nullptr,                                         // pNext
                (VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT |
                 VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT),  // flags
                DebugReportCallback,                            // pfnCallback
                nullptr                                         // pUserData
            };

            vkCreateDebugReportCallbackEXT(m_instance, &create_info, nullptr, &m_debugReportCallback);
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
        VK_STRUCTURE_TYPE_APPLICATION_INFO,  // sType
        nullptr,                             // pNext
        "Engine",                            // pApplicationName
        VK_MAKE_VERSION(1, 0, 0),            // applicationVersion
        "Engine",                            // pEngineName
        VK_MAKE_VERSION(1, 0, 0),            // engineVersion
        VK_MAKE_VERSION(1, 0, 0)             // apiVersion
    };

    // Check that all the instance extensions are supported
    if (!checkInstanceExtensionsSupport()) {
        LogFatal(sTag, "Error instance extensions");
        return false;
    };

    // Define all the information for the instance
    VkInstanceCreateInfo create_info = {
        VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,            // sType
        nullptr,                                           // pNext
        VkInstanceCreateFlags(),                           //  flags
        &appInfo,                                          //  pApplicationInfo
        static_cast<uint32>(m_validationLayers.size()),    //  enabledLayerCount
        m_validationLayers.data(),                         //  ppEnabledLayerNames
        static_cast<uint32>(m_instanceExtensions.size()),  //  enabledExtensionCount
        m_instanceExtensions.data()                        //  ppEnabledExtensionNames
    };

    // Create the Vulkan instance based on the provided info
    VkResult result = vkCreateInstance(&create_info, nullptr, &m_instance);
    if (result != VK_SUCCESS) {
        LogFatal(sTag, "Failed to create Vulkan instance");
        return false;
    }

    return true;
}

bool Vk_Context::createDevice() {
    VkResult result = VK_SUCCESS;

    // Query all the avaliable physical devices
    uint32 physical_devices_count = 0;
    Vector<VkPhysicalDevice> physical_devices;

    result = vkEnumeratePhysicalDevices(m_instance, &physical_devices_count, nullptr);
    if (physical_devices_count > 0 && result == VK_SUCCESS) {
        physical_devices.resize(physical_devices_count);
        result = vkEnumeratePhysicalDevices(m_instance, &physical_devices_count, physical_devices.data());
    }
    if (physical_devices_count == 0 || result != VK_SUCCESS) {
        LogFatal(sTag, "Error querying physical devices");
        return false;
    }

    // Check all the queried physical devices for one with the required
    // caracteristics and avaliable queues
    for (auto& physical_device : physical_devices) {
        if (selectPhysicalDevice(physical_device)) {
            break;
        }
    }
    if (!m_physicalDevice.handle || m_graphicsQueue.familyIndex == UINT32_MAX) {
        LogFatal("Vk_Core", "No physical device that supports the required caracteristics");
        return false;
    }

    // Define the queue families information
    Vector<float> queue_priorities = {1.0F};
    Vector<VkDeviceQueueCreateInfo> queue_create_infos;
    queue_create_infos.push_back({
        VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,    // sType
        nullptr,                                       // pNext
        VkDeviceQueueCreateFlags(),                    // flags
        m_graphicsQueue.familyIndex,                   // queueFamilyIndex
        static_cast<uint32>(queue_priorities.size()),  // queueCount
        queue_priorities.data()                        // pQueuePriorities
    });

    // TODO: Configure this in runtime
    m_enabledFeatures = {};
    m_enabledFeatures.samplerAnisotropy = m_physicalDevice.features.samplerAnisotropy;

    // Define all the information for the logical device
    VkDeviceCreateInfo device_create_info = {
        VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,            // sType
        nullptr,                                         // pNext
        VkDeviceCreateFlags(),                           // flags
        static_cast<uint32>(queue_create_infos.size()),  // queueCreateInfoCount
        queue_create_infos.data(),                       // pQueueCreateInfos
        static_cast<uint32>(m_validationLayers.size()),  // enabledLayerCount
        m_validationLayers.data(),                       // ppEnabledLayerNames
        static_cast<uint32>(m_deviceExtensions.size()),  // enabledExtensionCount
        m_deviceExtensions.data(),                       // ppEnabledExtensionNames
        &m_enabledFeatures                               // pEnabledFeatures
    };

    // Create the logical device based on the retrived info
    result = vkCreateDevice(m_physicalDevice.handle, &device_create_info, nullptr, &m_device);
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

    uint32 major_version = VK_VERSION_MAJOR(properties.apiVersion);
    // uint32 minor_version = VK_VERSION_MINOR(properties.apiVersion);
    // uint32 patch_version = VK_VERSION_PATCH(properties.apiVersion);

    if (major_version < 1 && properties.limits.maxImageDimension2D < 4096) {
        LogError(sTag,
                 "Physical device {} doesn't support required "
                 "parameters"_format(properties.deviceName));
        return false;
    }

    // Check if the physical device support the required extensions
    uint32 extensions_count = 0;
    Vector<VkExtensionProperties> available_extensions;

    result = vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensions_count, nullptr);
    if (result == VK_SUCCESS && extensions_count > 0) {
        available_extensions.resize(extensions_count);
        result =
            vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensions_count, &available_extensions[0]);
    }

    // Check that the avaliable extensions could be retreived
    if (result != VK_SUCCESS || extensions_count == 0) {
        LogError(sTag,
                 "Error occurred during physical device {} extensions "
                 "enumeration"_format(properties.deviceName));
        return false;
    }

    // Check that all the required device extensions exists
    for (auto& m_deviceExtension : m_deviceExtensions) {
        if (!CheckExtensionAvailability(m_deviceExtension, available_extensions)) {
            LogError(sTag,
                     "Physical device {} doesn't support extension "
                     "named \"{}\""_format(properties.deviceName, m_deviceExtension));
            return false;
        }
    }

    // Retreive all the queue families properties
    uint32 queue_families_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queue_families_count, nullptr);
    if (queue_families_count == 0) {
        LogError(sTag,
                 "Physical device {} doesn't have any queue "
                 "families"_format(properties.deviceName));
        return false;
    }
    Vector<VkQueueFamilyProperties> queue_family_properties(queue_families_count);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queue_families_count, queue_family_properties.data());

    // Find a queue family that supports graphics queue
    uint32 graphics_queue_family_index = UINT32_MAX;
    for (uint32 i = 0; i < queue_families_count; i++) {
        // Select first queue that supports graphics
        if (queue_family_properties[i].queueCount > 0 &&
            queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            graphics_queue_family_index = i;
        }
    }

    // If this device doesn't support queues with graphics and present
    // capabilities don't use it
    if (graphics_queue_family_index == UINT32_MAX) {
        LogError(sTag,
                 "Could not find queue family with required properties "
                 "on physical device: {}"_format(properties.deviceName));
        return false;
    }

    // Set the PhysicalDeviceProperties
    m_physicalDevice.handle = physicalDevice;
    m_physicalDevice.properties = properties;
    m_physicalDevice.features = features;

    // Set the graphical QueueProperties
    m_graphicsQueue.familyIndex = graphics_queue_family_index;
    m_graphicsQueue.properties = queue_family_properties[m_graphicsQueue.familyIndex];

    return true;
}

bool Vk_Context::checkValidationLayerSupport() const {
    VkResult result = VK_SUCCESS;

    // Get the avaliable layers
    uint32 layer_count = 0;
    Vector<VkLayerProperties> avaliable_layers;
    result = vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
    if (layer_count > 0 && result == VK_SUCCESS) {
        avaliable_layers.resize(layer_count);
        result = vkEnumerateInstanceLayerProperties(&layer_count, avaliable_layers.data());
    }

    // Check that the avaliable layers could be retreived
    if (layer_count == 0 || result != VK_SUCCESS) {
        LogError(sTag, "Error occurred during validation layers enumeration");
        return false;
    }

    Vector<const char*> available_layer_list = avaliable_layers.map([](const auto& layer) { return layer.layerName; });
    LogInfo(sTag, "Available Layers: {}"_format(available_layer_list));

    // Check that all the validation layers exists
    for (const auto& requested_layer : m_validationLayers) {
        if (!CheckLayerAvailability(requested_layer, avaliable_layers)) {
            LogError(sTag, "Could not find validation layer named: {}"_format(requested_layer));
            return false;
        }
    }

    return true;
}

bool Vk_Context::checkInstanceExtensionsSupport() const {
    VkResult result = VK_SUCCESS;

    // Get the avaliable extensions
    uint32 extensions_count = 0;
    Vector<VkExtensionProperties> available_extensions;
    result = vkEnumerateInstanceExtensionProperties(nullptr, &extensions_count, nullptr);
    if (extensions_count > 0 && result == VK_SUCCESS) {
        available_extensions.resize(extensions_count);
        result = vkEnumerateInstanceExtensionProperties(nullptr, &extensions_count, available_extensions.data());
    }

    // Check that the avaliable extensions could be retreived
    if (extensions_count == 0 || result != VK_SUCCESS) {
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
                 "Neither the 'VK_KHR_xlib_surface' extension nor the "
                 "'VK_KHR_xcb_surface' extension are supported by the "
                 "current system.");
        return false;
    }
#endif

    // Check that all the required instance extensions exists
    bool all_extensions_found = true;
    for (const char* instance_extension : m_instanceExtensions) {
#if defined(SDL_VIDEO_DRIVER_X11)
        if (!strcmp(x11_extension_to_ignore, instance_extension)) {
            continue;
        }
#endif
        if (!CheckExtensionAvailability(instance_extension, available_extensions)) {
            LogError(sTag,
                     "Could not find instance extension "
                     "named: {}"_format(instance_extension));
            all_extensions_found = false;
        }
    }

    return all_extensions_found;
}

bool Vk_Context::createVulkanCommandPool(QueueParameters& queue, VkCommandPool* cmdPool) {
    VkResult result = VK_SUCCESS;

    // Create the pool for the command buffers
    VkCommandPoolCreateInfo cmd_pool_create_info = {
        VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,                                                // sType
        nullptr,                                                                                   // pNext
        (VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT),  // flags
        queue.familyIndex                                                                          // queueFamilyIndex
    };

    result = vkCreateCommandPool(m_device, &cmd_pool_create_info, nullptr, cmdPool);
    if (result != VK_SUCCESS) {
        LogError(sTag, "Could not create a command pool");
        return false;
    }

    return true;
}

bool Vk_Context::createUboDescriptorPool() {
    VkResult result = VK_SUCCESS;

    std::array<VkDescriptorPoolSize, 2> pool_sizes = {{
        {
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,  // type
            sMaxUBODescriptorSets               // descriptorCount
        },
        {
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,  // type
            sMaxUBODynamicDescriptorSets                // descriptorCount
        },
    }};

    uint32 max_sets = 0;
    for (VkDescriptorPoolSize& pool_size : pool_sizes) {
        max_sets += pool_size.descriptorCount;
    }

    VkDescriptorPoolCreateInfo descriptor_pool_create_info = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,  // sType
        nullptr,                                        // pNext
        0,                                              // flags
        max_sets,                                       // maxSets
        static_cast<uint32>(pool_sizes.size()),         // poolSizeCount
        pool_sizes.data()                               // pPoolSizes
    };

    result = vkCreateDescriptorPool(m_device, &descriptor_pool_create_info, nullptr, &m_uboDescriptorPool);
    if (result != VK_SUCCESS) {
        LogError(sTag, "Could not create descriptor pool");
        return false;
    }

    return true;
}

}  // namespace engine
