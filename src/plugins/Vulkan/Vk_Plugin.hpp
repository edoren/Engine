#pragma once

#include <Core/Plugin.hpp>

#include "Vk_Config.hpp"

namespace engine {

class String;

class Vk_Renderer;

class VULKAN_PLUGIN_API Vk_Plugin : public Plugin {
public:
    Vk_Plugin();

    /// @copydoc Plugin::GetName
    const String& GetName() const;

    /// @copydoc Plugin::Install
    void Install();

    /// @copydoc Plugin::Initialize
    void Initialize();

    /// @copydoc Plugin::Shutdown
    void Shutdown();

    /// @copydoc Plugin::Uninstall
    void Uninstall();

protected:
    Vk_Renderer* m_renderer;
};

}  // namespace engine
