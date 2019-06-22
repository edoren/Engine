#pragma once

#include <Renderer/Renderer.hpp>

#include "Vk_Config.hpp"
#include "Vk_Context.hpp"
#include "Vk_RenderWindow.hpp"

namespace engine {

class VULKAN_PLUGIN_API Vk_Renderer : public Renderer {
public:
    Vk_Renderer();

    ~Vk_Renderer();

    bool Initialize() override;

    void Shutdown() override;

    void AdvanceFrame() override;

    const String& GetName() const override;

private:
    std::unique_ptr<Vk_Context> m_context;
};

}  // namespace engine
