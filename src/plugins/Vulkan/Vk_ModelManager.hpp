#pragma once

#include <Renderer/ModelManager.hpp>

#include "Vk_Config.hpp"
#include "Vk_Dependencies.hpp"

#include <memory>

namespace engine {

class String;

class Vk_Texture2D;

class VULKAN_PLUGIN_API Vk_ModelManager : public ModelManager {
public:
    Vk_ModelManager();

    ~Vk_ModelManager() override;

    /**
     * @copydoc Main::GetInstance
     */
    static Vk_ModelManager& GetInstance();

    /**
     * @copydoc Main::GetInstance
     */
    static Vk_ModelManager* GetInstancePtr();

protected:
    std::unique_ptr<Model> createModel() override;
    std::unique_ptr<Mesh> createMesh() override;
};

}  // namespace engine
