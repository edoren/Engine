#include <System/LogManager.hpp>

#include "Vk_Context.hpp"
#include "Vk_Mesh.hpp"
#include "Vk_ModelManager.hpp"
#include "Vk_Texture2D.hpp"

#include <memory>

namespace engine {

namespace {

const String sTag("Vk_ModelManager");

Vk_ModelManager* sDerivedInstance = nullptr;

}  // namespace

Vk_ModelManager& Vk_ModelManager::GetInstance() {
    assert(sDerivedInstance);
    return (*sDerivedInstance);
}

Vk_ModelManager* Vk_ModelManager::GetInstancePtr() {
    return sDerivedInstance;
}

Vk_ModelManager::Vk_ModelManager() {
    ModelManager& baseInstance = ModelManager::GetInstance();
    sDerivedInstance = reinterpret_cast<Vk_ModelManager*>(&baseInstance);
}

Vk_ModelManager::~Vk_ModelManager() = default;

std::unique_ptr<Model> Vk_ModelManager::createModel() {
    return std::make_unique<Model>();
}

std::unique_ptr<Mesh> Vk_ModelManager::createMesh() {
    return std::make_unique<Vk_Mesh>();
}

}  // namespace engine
