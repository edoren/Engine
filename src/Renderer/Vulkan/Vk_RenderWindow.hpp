#pragma once

#include <Util/Prerequisites.hpp>

#include <Math/Math.hpp>
#include <Renderer/RenderWindow.hpp>
#include <System/String.hpp>

#include "Vk_Config.hpp"
#include "Vk_Dependencies.hpp"

namespace engine {

class VULKAN_PLUGIN_API Vk_RenderWindow : public RenderWindow {
public:
    Vk_RenderWindow();
    ~Vk_RenderWindow();

    ////////////////////////////////////////////////////////////////
    // Overridden
    virtual bool Create(const String& name, const math::ivec2& size);

    virtual void Destroy(void);

    virtual void Reposition(int left, int top);

    virtual void Resize(int width, int height);

    virtual void SetFullScreen(bool fullscreen, bool is_fake);

    virtual void SetVSyncEnabled(bool vsync);

    virtual void SwapBuffers();  // RenderTarget

    virtual void Clear(const Color& color);  // Render Target

    virtual bool IsVisible();
    ////////////////////////////////////////////////////////////////

private:
    bool CreateVulkanInstance();
    bool CreateVulkanDevice();
    bool CreateVulkanSurface();

    bool CheckVulkanValidationLayerSupport() const;

    bool CheckVulkanInstanceExtensionsSupport() const;

    bool CheckPhysicalDevice(vk::PhysicalDevice& physical_device,
                             uint32_t& selected_graphics_queue_family_index,
                             uint32_t& selected_present_queue_family_index);

private:
    SDL_Window* m_window;

    vk::Instance m_instance;
    vk::SurfaceKHR m_surface;
    vk::Device m_device;

    uint32_t m_queue_family_index;
    uint32_t m_graphics_queue_family_index;
    uint32_t m_present_queue_family_index;

    bool m_validation_layers_enabled;

    std::vector<const char*> m_validation_layers;

    std::vector<const char*> m_instance_extensions;
    std::vector<const char*> m_device_extensions;
};

}  // namespace engine
