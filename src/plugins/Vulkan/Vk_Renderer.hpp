#pragma once

#include <Renderer/Renderer.hpp>

#include "Vk_Config.hpp"
#include "Vk_Context.hpp"
#include "Vk_RenderWindow.hpp"

#include <memory>

namespace engine::plugin::vulkan {

class VULKAN_PLUGIN_API Vk_Renderer : public Renderer {
public:
    Vk_Renderer();

    ~Vk_Renderer() override;

    bool initialize() override;

    void shutdown() override;

    void advanceFrame() override;

    const String& getName() const override;

private:
    std::unique_ptr<Vk_Context> m_context;
};

}  // namespace engine::plugin::vulkan
