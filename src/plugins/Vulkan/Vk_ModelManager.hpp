#pragma once

#include <Renderer/ModelManager.hpp>

#include "Vk_Config.hpp"
#include "Vk_Dependencies.hpp"

namespace engine {

class String;

class Vk_Texture2D;

class VULKAN_PLUGIN_API Vk_ModelManager : public ModelManager {
public:
    Vk_ModelManager();

    ~Vk_ModelManager() override;

    static Vk_ModelManager& GetInstance();

    static Vk_ModelManager* GetInstancePtr();

protected:
    std::unique_ptr<Model> CreateModel() override;
    std::unique_ptr<Mesh> CreateMesh() override;
};

}  // namespace engine
