#pragma once

#include <Core/Plugin.hpp>

#include "Vk_Config.hpp"

namespace engine {

class String;

class Vk_Renderer;

class VULKAN_PLUGIN_API Vk_Plugin : public Plugin {
public:
    Vk_Plugin();

    const String& getName() const override;

    void install() override;

    void initialize() override;

    void shutdown() override;

    void uninstall() override;

protected:
    Vk_Renderer* m_renderer;
};

}  // namespace engine
