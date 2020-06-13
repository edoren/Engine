#include <Renderer/ModelManager.hpp>
#include <System/FileSystem.hpp>
#include <System/LogManager.hpp>
#include <System/StringFormat.hpp>

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
    m_model_ref_count.clear();
    m_models.clear();
}

Model* ModelManager::LoadFromFile(const String& basename) {
    auto it = m_name_map.find(basename);
    if (it != m_name_map.end()) {
        m_model_ref_count[it->second] += 1;
        return it->second;
    }

    LogDebug(sTag, "Loading model: " + basename);

    m_models.emplace_back(CreateModel());
    m_models.back()->LoadModel(basename);
    m_name_map[basename] = m_models.back().get();
    m_model_ref_count[m_models.back().get()] = 1;

    return m_models.back().get();
}

void ModelManager::Unload(Model* model) {
    auto& ref_count = m_model_ref_count[model];
    ref_count -= 1;

    if (ref_count == 0) {
        auto found_it =
            std::find_if(m_models.begin(), m_models.end(), [&model](auto& val) { return val.get() == model; });
        auto found_name_it =
            std::find_if(m_name_map.begin(), m_name_map.end(), [&model](auto& pair) { return pair.second == model; });

        found_it->reset(nullptr);

        m_name_map.erase(found_name_it->first);
        m_model_ref_count.erase(model);
        m_models.erase(found_it);

        LogDebug(sTag, "Unloading model: {}"_format(found_name_it->first));
    }
}

void ModelManager::UnloadFromFile(const String& basename) {
    auto it = m_name_map.find(basename);
    if (it == m_name_map.end()) {
        return;
    }
    Unload(it->second);
}

}  // namespace engine
