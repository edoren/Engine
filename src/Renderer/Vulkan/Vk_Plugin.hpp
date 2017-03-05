#pragma once

#include <Core/Plugin.hpp>
#include <System/String.hpp>

#include "Vk_Renderer.hpp"
#include "Vk_Config.hpp"

namespace engine {

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
