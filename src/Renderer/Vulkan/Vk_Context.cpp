#include <System/LogManager.hpp>
#include <System/StringFormat.hpp>

#include "Vk_Context.hpp"

namespace engine {

namespace {

bool CheckExtensionAvailability(const char* str,
                                const std::vector<VkExtensionProperties>& vec) {
    for (size_t i = 0; i < vec.size(); i++) {
        if (!strcmp(str, vec[i].extensionName)) {
            return true;
        }
    }
    return false;
}

bool CheckLayerAvailability(const char* str,
                            const std::vector<VkLayerProperties>& vec) {
    for (size_t i = 0; i < vec.size(); i++) {
        if (!strcmp(str, vec[i].layerName)) {
            return true;
        }
    }
    return false;
}

}  // namespace

template <>
Vk_Context* Singleton<Vk_Context>::s_instance = nullptr;

Vk_Context& Vk_Context::GetInstance() {
    assert(s_instance);
    return (*s_instance);
}

Vk_Context* Vk_Context::GetInstancePtr() {
    return s_instance;
}

Vk_Context::Vk_Context()
      : m_instance(VK_NULL_HANDLE), m_device(VK_NULL_HANDLE) {}

Vk_Context::~Vk_Context() {
    Shutdown();
}

bool Vk_Context::Initialize() {
    // Add the required validation layers
    if (m_validation_layers_enabled) {
        m_validation_layers.push_back("VK_LAYER_LUNARG_standard_validation");
    }

    // Add the required Instance extensions
    m_instance_extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    m_instance_extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_XCB_KHR)
    m_instance_extensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
    m_instance_extensions.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#endif
    if (m_validation_layers_enabled) {
        m_instance_extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    }

    // Add the required Device extensions
    m_device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    // Check the validation layers support
    if (m_validation_layers_enabled && !CheckValidationLayerSupport()) {
        LogFatal("Vk_Core", "Validation layers requested, but not available");
        return false;
    }

    CreateInstance();
    CreateDevice();

    return true;
}

void Vk_Context::Shutdown() {
    if (m_device) vkDestroyDevice(m_device, nullptr);
    if (m_instance) vkDestroyInstance(m_instance, nullptr);
    ;
    m_device = nullptr;
    m_instance = nullptr;
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

bool Vk_Context::CreateInstance() {
    // Define the application information
    VkApplicationInfo appInfo = {
        VK_STRUCTURE_TYPE_APPLICATION_INFO,  // sType
        nullptr,                             // pNext
        "Engine",                            // pApplicationName
        VK_MAKE_VERSION(1, 0, 0),            // applicationVersion
        "Engine",                            // pEngineName
        VK_MAKE_VERSION(1, 0, 0),            // engineVersion
        VK_API_VERSION_1_0                   // apiVersion
    };

    // Check that all the instance extensions are supported
    if (!CheckInstanceExtensionsSupport()) {
        LogFatal("Vk_Core", "Error instance extensions");
        return false;
    };

    // Define all the information for the instance
    VkInstanceCreateInfo create_info = {
        VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,           // sType
        nullptr,                                          // pNext
        VkInstanceCreateFlags(),                          //  flags
        &appInfo,                                         //  pApplicationInfo
        static_cast<uint32>(m_validation_layers.size()),  //  enabledLayerCount
        m_validation_layers.data(),  //  ppEnabledLayerNames
        static_cast<uint32>(
            m_instance_extensions.size()),  //  enabledExtensionCount
        m_instance_extensions.data()        //  ppEnabledExtensionNames
    };

    // Create the Vulkan instance based on the provided info
    VkResult result = vkCreateInstance(&create_info, nullptr, &m_instance);
    if (result != VK_SUCCESS) {
        LogFatal("Vk_Core", "Failed to create Vulkan instance");
        return false;
    }

    return true;
}

bool Vk_Context::CreateDevice() {
    VkResult result = VK_SUCCESS;

    // Query all the avaliable physical devices
    uint32 physical_devices_count = 0;
    std::vector<VkPhysicalDevice> physical_devices;

    result = vkEnumeratePhysicalDevices(m_instance, &physical_devices_count,
                                        nullptr);
    if (physical_devices_count > 0 && result == VK_SUCCESS) {
        physical_devices.resize(physical_devices_count);
        result = vkEnumeratePhysicalDevices(m_instance, &physical_devices_count,
                                            physical_devices.data());
    }
    if (physical_devices_count == 0 || result != VK_SUCCESS) {
        LogFatal("Vk_Core", "Error querying physical devices");
        return false;
    }

    // Check all the queried physical devices for one with the required
    // caracteristics and avaliable queues
    for (size_t i = 0; i < physical_devices.size(); i++) {
        if (SelectPhysicalDevice(physical_devices[i])) break;
    }
    if (!m_physical_device.handle || m_graphics_queue.index == UINT32_MAX) {
        LogFatal(
            "Vk_Core",
            "No physical device that supports the required caracteristics");
        return false;
    }

    // Define the queue families information
    std::vector<float> queue_priorities = {1.0f};
    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
    queue_create_infos.push_back({
        VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,    // sType
        nullptr,                                       // pNext
        VkDeviceQueueCreateFlags(),                    // flags
        m_graphics_queue.index,                        // queueFamilyIndex
        static_cast<uint32>(queue_priorities.size()),  // queueCount
        queue_priorities.data()                        // pQueuePriorities
    });

    // Define all the information for the logical device
    VkDeviceCreateInfo device_create_info = {
        VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,            // sType
        nullptr,                                         // pNext
        VkDeviceCreateFlags(),                           // flags
        static_cast<uint32>(queue_create_infos.size()),  // queueCreateInfoCount
        queue_create_infos.data(),                       // pQueueCreateInfos
        static_cast<uint32>(m_validation_layers.size()),  // enabledLayerCount
        m_validation_layers.data(),                       // ppEnabledLayerNames
        static_cast<uint32>(
            m_device_extensions.size()),  // enabledExtensionCount
        m_device_extensions.data(),       // ppEnabledExtensionNames
        nullptr                           // pEnabledFeatures
    };

    // Create the logical device based on the retrived info
    result = vkCreateDevice(m_physical_device.handle, &device_create_info,
                            nullptr, &m_device);
    if (result != VK_SUCCESS) {
        LogFatal("Vk_Core", "Could not create Vulkan device");
        return false;
    }

    // Get the Queue handles
    vkGetDeviceQueue(m_device, m_graphics_queue.index, 0,
                     &m_graphics_queue.handle);

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
        LogError("Vk_Core",
                 "Physical device {} doesn't support required "
                 "parameters"_format(properties.deviceName));
        return false;
    }

    // Check if the physical device support the required extensions
    uint32 extensions_count = 0;
    std::vector<VkExtensionProperties> available_extensions;

    result = vkEnumerateDeviceExtensionProperties(physical_device, nullptr,
                                                  &extensions_count, nullptr);
    if (result == VK_SUCCESS && extensions_count > 0) {
        available_extensions.resize(extensions_count);
        result = vkEnumerateDeviceExtensionProperties(physical_device, nullptr,
                                                      &extensions_count,
                                                      &available_extensions[0]);
    }

    // Check that the avaliable extensions could be retreived
    if (result != VK_SUCCESS || extensions_count == 0) {
        LogError("Vk_Core",
                 "Error occurred during physical device {} extensions "
                 "enumeration"_format(properties.deviceName));
        return false;
    }

    // Check that all the required device extensions exists
    for (size_t i = 0; i < m_device_extensions.size(); i++) {
        if (!CheckExtensionAvailability(m_device_extensions[i],
                                        available_extensions)) {
            LogError("Vk_Core",
                     "Physical device {} doesn't support extension "
                     "named \"{}\""_format(properties.deviceName,
                                           m_device_extensions[i]));
            return false;
        }
    }

    // Retreive all the queue families properties
    uint32 queue_families_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device,
                                             &queue_families_count, nullptr);
    if (queue_families_count == 0) {
        LogError("Vk_Core",
                 "Physical device {} doesn't have any queue "
                 "families"_format(properties.deviceName));
        return false;
    }
    std::vector<VkQueueFamilyProperties> queue_family_properties(
        queue_families_count);
    vkGetPhysicalDeviceQueueFamilyProperties(
        physical_device, &queue_families_count, queue_family_properties.data());

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
        LogError("Vk_Core",
                 "Could not find queue family with required properties "
                 "on physical device: {}"_format(properties.deviceName));
        return false;
    }

    // Set the PhysicalDeviceProperties
    m_physical_device.handle = physical_device;
    m_physical_device.properties = properties;
    m_physical_device.features = features;

    // Set the graphical QueueProperties
    m_graphics_queue.index = graphics_queue_family_index;
    m_graphics_queue.properties =
        queue_family_properties[m_graphics_queue.index];

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
        result = vkEnumerateInstanceLayerProperties(&layer_count,
                                                    avaliable_layers.data());
    }

    // Check that the avaliable layers could be retreived
    if (layer_count == 0 || result != VK_SUCCESS) {
        LogError("Vk_Core",
                 "Error occurred during validation layers enumeration");
        return false;
    }

    // Check that all the validation layers exists
    for (size_t i = 0; i < m_validation_layers.size(); i++) {
        if (!CheckLayerAvailability(m_validation_layers[i], avaliable_layers)) {
            LogError("Vk_Core",
                     "Could not find validation layer "
                     "named: {}"_format(m_validation_layers[i]));
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
    result = vkEnumerateInstanceExtensionProperties(nullptr, &extensions_count,
                                                    nullptr);
    if (extensions_count > 0 && result == VK_SUCCESS) {
        available_extensions.resize(extensions_count);
        result = vkEnumerateInstanceExtensionProperties(
            nullptr, &extensions_count, available_extensions.data());
    }

    // Check that the avaliable extensions could be retreived
    if (extensions_count == 0 || result != VK_SUCCESS) {
        LogError("Vk_Core",
                 "Error occurred during instance extensions enumeration");
        return false;
    }

    // Check that all the required instance extensions exists
    for (size_t i = 0; i < m_instance_extensions.size(); i++) {
        if (!CheckExtensionAvailability(m_instance_extensions[i],
                                        available_extensions)) {
            LogError("Vk_Core",
                     "Could not find instance extension "
                     "named: {}"_format(m_instance_extensions[i]));
            return false;
        }
    }

    return true;
}

}  // namespace engine
