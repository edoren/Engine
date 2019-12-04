#include <System/LogManager.hpp>
#include <System/StringFormat.hpp>

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

bool CheckExtensionAvailability(const char* str, const std::vector<VkExtensionProperties>& vec) {
    for (size_t i = 0; i < vec.size(); i++) {
        if (!strcmp(str, vec[i].extensionName)) {
            return true;
        }
    }
    return false;
}

bool CheckLayerAvailability(const char* str, const std::vector<VkLayerProperties>& vec) {
    for (size_t i = 0; i < vec.size(); i++) {
        if (!strcmp(str, vec[i].layerName)) {
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
        m_graphics_queue(),
        m_graphics_queue_cmd_pool(VK_NULL_HANDLE),
        m_ubo_descriptor_pool(VK_NULL_HANDLE),
        m_debug_report_callback(VK_NULL_HANDLE),
#ifdef ENGINE_DEBUG
        m_validation_layers_enabled(true),
#else
        m_validation_layers_enabled(false),
#endif
        m_validation_layers(),
        m_instance_extensions(),
        m_device_extensions() {
}

Vk_Context::~Vk_Context() {
    Shutdown();
}

bool Vk_Context::Initialize() {
    // Add the required validation layers
    if (m_validation_layers_enabled) {
#if PLATFORM_IS(PLATFORM_ANDROID)
        m_validation_layers = {"VK_LAYER_GOOGLE_threading", "VK_LAYER_LUNARG_parameter_validation",
                               "VK_LAYER_LUNARG_object_tracker", "VK_LAYER_LUNARG_core_validation",
                               "VK_LAYER_GOOGLE_unique_objects"};
#else
        m_validation_layers = {"VK_LAYER_KHRONOS_validation"};
#endif
    }

    // Add the required Instance extensions
    m_instance_extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
#if defined(SDL_VIDEO_DRIVER_WINDOWS)
    m_instance_extensions.push_back("VK_KHR_win32_surface");
#endif
#if defined(SDL_VIDEO_DRIVER_X11)
    m_instance_extensions.push_back("VK_KHR_xcb_surface");
    m_instance_extensions.push_back("VK_KHR_xlib_surface");
#endif
#if defined(SDL_VIDEO_DRIVER_WAYLAND)
    m_instance_extensions.push_back("VK_KHR_wayland_surface");
#endif
#if defined(SDL_VIDEO_DRIVER_MIR)
    m_instance_extensions.push_back("VK_KHR_mir_surface");
#endif
#if defined(SDL_VIDEO_DRIVER_ANDROID)
    m_instance_extensions.push_back("VK_KHR_android_surface");
#endif
#if defined(SDL_VIDEO_DRIVER_COCOA)
    m_instance_extensions.push_back("VK_MVK_macos_surface");
#endif
#if defined(SDL_VIDEO_DRIVER_UIKIT)
    m_instance_extensions.push_back("VK_MVK_ios_surface");
#endif
    if (m_instance_extensions.size() < 2) {
        LogFatal(sTag, "Platform does not has a supported surface extension");
    }
    if (m_validation_layers_enabled) {
        m_instance_extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    }

    // Add the required Device extensions
    m_device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    // Check the validation layers support
    if (m_validation_layers_enabled && !CheckValidationLayerSupport()) {
        LogFatal(sTag, "Validation layers requested, but not available");
        return false;
    }

    CreateInstance();
    CreateDevice();

    if (m_validation_layers_enabled && !m_debug_report_callback) {
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

            vkCreateDebugReportCallbackEXT(m_instance, &create_info, nullptr, &m_debug_report_callback);
        }
    }

    if (!CreateVulkanCommandPool(m_graphics_queue, &m_graphics_queue_cmd_pool)) {
        return false;
    }

    if (!CreateUBODescriptorPool()) {
        LogError(sTag, "Could not create the UBO descriptor pool");
        return false;
    }

    return true;
}

void Vk_Context::Shutdown() {
    if (m_device) {
        if (m_graphics_queue_cmd_pool) {
            vkDestroyCommandPool(m_device, m_graphics_queue_cmd_pool, nullptr);
            m_graphics_queue_cmd_pool = VK_NULL_HANDLE;
        }

        if (m_ubo_descriptor_pool) {
            vkDestroyDescriptorPool(m_device, m_ubo_descriptor_pool, nullptr);
            m_ubo_descriptor_pool = VK_NULL_HANDLE;
        }

        vkDestroyDevice(m_device, nullptr);
        m_device = VK_NULL_HANDLE;
    }

    if (m_validation_layers_enabled && m_debug_report_callback) {
        PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT;
        vkDestroyDebugReportCallbackEXT = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(
            vkGetInstanceProcAddr(m_instance, "vkDestroyDebugReportCallbackEXT"));

        if (vkDestroyDebugReportCallbackEXT) {
            vkDestroyDebugReportCallbackEXT(m_instance, m_debug_report_callback, nullptr);
        }
    }

    if (m_instance) {
        vkDestroyInstance(m_instance, nullptr);
        m_instance = VK_NULL_HANDLE;
    }
}

VkInstance& Vk_Context::GetVulkanInstance() {
    return m_instance;
}

VkDevice& Vk_Context::GetVulkanDevice() {
    return m_device;
}

PhysicalDeviceParameters& Vk_Context::GetPhysicalDevice() {
    return m_physical_device;
}

QueueParameters& Vk_Context::GetGraphicsQueue() {
    return m_graphics_queue;
}

VkCommandPool& Vk_Context::GetGraphicsQueueCmdPool() {
    return m_graphics_queue_cmd_pool;
}

VkDescriptorPool& Vk_Context::GetUBODescriptorPool() {
    return m_ubo_descriptor_pool;
}

const VkPhysicalDeviceFeatures& Vk_Context::GetEnabledFeatures() {
    return m_enabled_features;
}

bool Vk_Context::CreateInstance() {
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
    if (!CheckInstanceExtensionsSupport()) {
        LogFatal(sTag, "Error instance extensions");
        return false;
    };

    // Define all the information for the instance
    VkInstanceCreateInfo create_info = {
        VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,             // sType
        nullptr,                                            // pNext
        VkInstanceCreateFlags(),                            //  flags
        &appInfo,                                           //  pApplicationInfo
        static_cast<uint32>(m_validation_layers.size()),    //  enabledLayerCount
        m_validation_layers.data(),                         //  ppEnabledLayerNames
        static_cast<uint32>(m_instance_extensions.size()),  //  enabledExtensionCount
        m_instance_extensions.data()                        //  ppEnabledExtensionNames
    };

    // Create the Vulkan instance based on the provided info
    VkResult result = vkCreateInstance(&create_info, nullptr, &m_instance);
    if (result != VK_SUCCESS) {
        LogFatal(sTag, "Failed to create Vulkan instance");
        return false;
    }

    return true;
}

bool Vk_Context::CreateDevice() {
    VkResult result = VK_SUCCESS;

    // Query all the avaliable physical devices
    uint32 physical_devices_count = 0;
    std::vector<VkPhysicalDevice> physical_devices;

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
    for (size_t i = 0; i < physical_devices.size(); i++) {
        if (SelectPhysicalDevice(physical_devices[i])) break;
    }
    if (!m_physical_device.handle || m_graphics_queue.family_index == UINT32_MAX) {
        LogFatal("Vk_Core", "No physical device that supports the required caracteristics");
        return false;
    }

    // Define the queue families information
    std::vector<float> queue_priorities = {1.0f};
    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
    queue_create_infos.push_back({
        VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,    // sType
        nullptr,                                       // pNext
        VkDeviceQueueCreateFlags(),                    // flags
        m_graphics_queue.family_index,                 // queueFamilyIndex
        static_cast<uint32>(queue_priorities.size()),  // queueCount
        queue_priorities.data()                        // pQueuePriorities
    });

    // TODO: Configure this in runtime
    m_enabled_features = {};
    m_enabled_features.samplerAnisotropy = m_physical_device.features.samplerAnisotropy;

    // Define all the information for the logical device
    VkDeviceCreateInfo device_create_info = {
        VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,             // sType
        nullptr,                                          // pNext
        VkDeviceCreateFlags(),                            // flags
        static_cast<uint32>(queue_create_infos.size()),   // queueCreateInfoCount
        queue_create_infos.data(),                        // pQueueCreateInfos
        static_cast<uint32>(m_validation_layers.size()),  // enabledLayerCount
        m_validation_layers.data(),                       // ppEnabledLayerNames
        static_cast<uint32>(m_device_extensions.size()),  // enabledExtensionCount
        m_device_extensions.data(),                       // ppEnabledExtensionNames
        &m_enabled_features                               // pEnabledFeatures
    };

    // Create the logical device based on the retrived info
    result = vkCreateDevice(m_physical_device.handle, &device_create_info, nullptr, &m_device);
    if (result != VK_SUCCESS) {
        LogFatal(sTag, "Could not create Vulkan device");
        return false;
    }

    // Get the Queue handles
    vkGetDeviceQueue(m_device, m_graphics_queue.family_index, 0, &m_graphics_queue.handle);

    return true;
}

bool Vk_Context::SelectPhysicalDevice(VkPhysicalDevice& physical_device) {
    VkResult result = VK_SUCCESS;

    // Check the PhysicalDevice properties and features
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceProperties(physical_device, &properties);
    vkGetPhysicalDeviceFeatures(physical_device, &features);

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
    std::vector<VkExtensionProperties> available_extensions;

    result = vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extensions_count, nullptr);
    if (result == VK_SUCCESS && extensions_count > 0) {
        available_extensions.resize(extensions_count);
        result =
            vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extensions_count, &available_extensions[0]);
    }

    // Check that the avaliable extensions could be retreived
    if (result != VK_SUCCESS || extensions_count == 0) {
        LogError(sTag,
                 "Error occurred during physical device {} extensions "
                 "enumeration"_format(properties.deviceName));
        return false;
    }

    // Check that all the required device extensions exists
    for (size_t i = 0; i < m_device_extensions.size(); i++) {
        if (!CheckExtensionAvailability(m_device_extensions[i], available_extensions)) {
            LogError(sTag,
                     "Physical device {} doesn't support extension "
                     "named \"{}\""_format(properties.deviceName, m_device_extensions[i]));
            return false;
        }
    }

    // Retreive all the queue families properties
    uint32 queue_families_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_families_count, nullptr);
    if (queue_families_count == 0) {
        LogError(sTag,
                 "Physical device {} doesn't have any queue "
                 "families"_format(properties.deviceName));
        return false;
    }
    std::vector<VkQueueFamilyProperties> queue_family_properties(queue_families_count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_families_count, queue_family_properties.data());

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
    m_physical_device.handle = physical_device;
    m_physical_device.properties = properties;
    m_physical_device.features = features;

    // Set the graphical QueueProperties
    m_graphics_queue.family_index = graphics_queue_family_index;
    m_graphics_queue.properties = queue_family_properties[m_graphics_queue.family_index];

    return true;
}

bool Vk_Context::CheckValidationLayerSupport() const {
    VkResult result = VK_SUCCESS;

    // Get the avaliable layers
    uint32 layer_count = 0;
    std::vector<VkLayerProperties> avaliable_layers;
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

    String available_layer_list;
    for (size_t i = 0; i < avaliable_layers.size(); i++) {
        if (i != 0) available_layer_list += ", ";
        available_layer_list += avaliable_layers[i].layerName;
    }
    LogInfo(sTag, "Available Layers: [{}]"_format(available_layer_list));

    // Check that all the validation layers exists
    for (auto& requested_layer : m_validation_layers) {
        if (!CheckLayerAvailability(requested_layer, avaliable_layers)) {
            LogError(sTag, "Could not find validation layer named: {}"_format(requested_layer));
            return false;
        }
    }

    return true;
}

bool Vk_Context::CheckInstanceExtensionsSupport() const {
    VkResult result = VK_SUCCESS;

    // Get the avaliable extensions
    uint32 extensions_count = 0;
    std::vector<VkExtensionProperties> available_extensions;
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
    for (const char* instance_extension : m_instance_extensions) {
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

bool Vk_Context::CreateVulkanCommandPool(QueueParameters& queue, VkCommandPool* cmd_pool) {
    VkResult result = VK_SUCCESS;

    // Create the pool for the command buffers
    VkCommandPoolCreateInfo cmd_pool_create_info = {
        VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,                                                // sType
        nullptr,                                                                                   // pNext
        (VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT),  // flags
        queue.family_index                                                                         // queueFamilyIndex
    };

    result = vkCreateCommandPool(m_device, &cmd_pool_create_info, nullptr, cmd_pool);
    if (result != VK_SUCCESS) {
        LogError(sTag, "Could not create a command pool");
        return false;
    }

    return true;
}

bool Vk_Context::CreateUBODescriptorPool() {
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

    result = vkCreateDescriptorPool(m_device, &descriptor_pool_create_info, nullptr, &m_ubo_descriptor_pool);
    if (result != VK_SUCCESS) {
        LogError(sTag, "Could not create descriptor pool");
        return false;
    }

    return true;
}

}  // namespace engine
