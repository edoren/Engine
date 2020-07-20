#pragma once

#include <Core/Plugin.hpp>

#include "Vk_Config.hpp"

namespace engine::plugin::vulkan {

class Vk_Renderer;

/**
 * @brief Vulkan plugin class
 */
class VULKAN_PLUGIN_API Vk_Plugin : public Plugin {
public:
    Vk_Plugin();

    const StringView& getName() const override;

    void install() override;

    void initialize() override;

    void shutdown() override;

    void uninstall() override;

protected:
    Vk_Renderer* m_renderer;
};

}  // namespace engine::plugin::vulkan
