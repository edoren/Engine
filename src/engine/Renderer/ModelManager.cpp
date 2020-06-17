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
    auto& refCount = m_modelRefCount[model];
    refCount -= 1;

    if (refCount == 0) {
        auto foundIt =
            std::find_if(m_models.begin(), m_models.end(), [&model](auto& val) { return val.get() == model; });
        auto foundNameIt =
            std::find_if(m_nameMap.begin(), m_nameMap.end(), [&model](auto& pair) { return pair.second == model; });

        foundIt->reset(nullptr);

        LogDebug(sTag, "Unloading model: {}"_format(foundNameIt->first));
        m_nameMap.erase(foundNameIt->first);
        m_modelRefCount.erase(model);
        m_models.erase(foundIt);
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
