#include <System/LogManager.hpp>

#include "Vk_Context.hpp"
#include "Vk_Mesh.hpp"
#include "Vk_ModelManager.hpp"
#include "Vk_Texture2D.hpp"

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
    ModelManager& base_instance = ModelManager::GetInstance();
    sDerivedInstance = reinterpret_cast<Vk_ModelManager*>(&base_instance);
}

Vk_ModelManager::~Vk_ModelManager() {}

std::unique_ptr<Model> Vk_ModelManager::createModel() {
    return std::make_unique<Model>();
}

std::unique_ptr<Mesh> Vk_ModelManager::createMesh() {
    return std::make_unique<Vk_Mesh>();
}

}  // namespace engine
