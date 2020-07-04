#pragma once

#include <Renderer/ModelManager.hpp>

#include "Vk_Config.hpp"
#include "Vk_Dependencies.hpp"

#include <memory>

namespace engine::plugin::vulkan {

class Vk_Texture2D;

class VULKAN_PLUGIN_API Vk_ModelManager : public ModelManager {
public:
    Vk_ModelManager();

    ~Vk_ModelManager() override;

    /**
     * @copydoc ModelManager::GetInstance
     */
    static Vk_ModelManager& GetInstance();

    /**
     * @copydoc ModelManager::GetInstance
     */
    static Vk_ModelManager* GetInstancePtr();

protected:
    std::unique_ptr<Model> createModel() override;
    std::unique_ptr<Mesh> createMesh() override;
};

}  // namespace engine::plugin::vulkan
