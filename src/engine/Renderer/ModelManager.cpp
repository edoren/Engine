
#include <Renderer/ModelManager.hpp>

namespace engine {

namespace {

const String sTag("ModelManager");

}  // namespace

template <>
ModelManager* Singleton<ModelManager>::sInstance = nullptr;

ModelManager& ModelManager::GetInstance() {
    return Singleton<ModelManager>::GetInstance();
}

ModelManager* ModelManager::GetInstancePtr() {
    return Singleton<ModelManager>::GetInstancePtr();
}

ModelManager::ModelManager() {}

ModelManager::~ModelManager() {}

void ModelManager::Initialize() {}

void ModelManager::Shutdown() {
    m_name_map.clear();
    m_index_map.clear();
    m_models.clear();
}

Model* ModelManager::LoadFromFile(const String& basename) {
    Model* model = GetModel(basename);
    if (model != nullptr) {
        return model;
    }

    m_models.emplace_back(new Model(basename));
    m_name_map[basename] = m_models.back().get();

    return m_models.back().get();
}

Model* ModelManager::GetModel(const String& name) {
    auto it = m_name_map.find(name);
    return (it != m_name_map.end()) ? it->second : nullptr;
}

}  // namespace engine
