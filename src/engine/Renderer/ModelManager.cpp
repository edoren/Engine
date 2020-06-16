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

ModelManager::ModelManager() = default;

ModelManager::~ModelManager() = default;

void ModelManager::initialize() {}

void ModelManager::shutdown() {
    m_nameMap.clear();
    m_modelRefCount.clear();
    m_models.clear();
}

Model* ModelManager::loadFromFile(const String& basename) {
    auto it = m_nameMap.find(basename);
    if (it != m_nameMap.end()) {
        m_modelRefCount[it->second] += 1;
        return it->second;
    }

    LogDebug(sTag, "Loading model: " + basename);

    m_models.emplace_back(createModel());
    m_models.back()->loadModel(basename);
    m_nameMap[basename] = m_models.back().get();
    m_modelRefCount[m_models.back().get()] = 1;

    return m_models.back().get();
}

void ModelManager::unload(Model* model) {
    auto& ref_count = m_modelRefCount[model];
    ref_count -= 1;

    if (ref_count == 0) {
        auto found_it =
            std::find_if(m_models.begin(), m_models.end(), [&model](auto& val) { return val.get() == model; });
        auto found_name_it =
            std::find_if(m_nameMap.begin(), m_nameMap.end(), [&model](auto& pair) { return pair.second == model; });

        found_it->reset(nullptr);

        m_nameMap.erase(found_name_it->first);
        m_modelRefCount.erase(model);
        m_models.erase(found_it);

        LogDebug(sTag, "Unloading model: {}"_format(found_name_it->first));
    }
}

void ModelManager::unloadFromFile(const String& basename) {
    auto it = m_nameMap.find(basename);
    if (it == m_nameMap.end()) {
        return;
    }
    unload(it->second);
}

}  // namespace engine
