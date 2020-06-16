#pragma once

#include <Core/Plugin.hpp>

#include "Vk_Config.hpp"

namespace engine {

class String;

class Vk_Renderer;

class VULKAN_PLUGIN_API Vk_Plugin : public Plugin {
public:
    Vk_Plugin();

    /// @copydoc Plugin::getName
    const String& getName() const override;

    /// @copydoc Plugin::install
    void install() override;

    /// @copydoc Plugin::initialize
    void initialize() override;

    /// @copydoc Plugin::shutdown
    void shutdown() override;

    /// @copydoc Plugin::uninstall
    void uninstall() override;

protected:
    Vk_Renderer* m_renderer;
};

}  // namespace engine
