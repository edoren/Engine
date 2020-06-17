#include <Renderer/SceneManager.hpp>

#include <Renderer/Model.hpp>
#include <Renderer/Scene.hpp>
#include <System/FileSystem.hpp>
#include <System/LogManager.hpp>

namespace engine {

namespace {

const String sTag("SceneManager");

const String sRootModelFolder("scenes");

uint32 sSceneIndex(0);

}  // namespace

template <>
SceneManager* Singleton<SceneManager>::sInstance = nullptr;

SceneManager& SceneManager::GetInstance() {
    return Singleton<SceneManager>::GetInstance();
}

SceneManager* SceneManager::GetInstancePtr() {
    return Singleton<SceneManager>::GetInstancePtr();
}

SceneManager::SceneManager() = default;

SceneManager::~SceneManager() = default;

void SceneManager::initialize() {}

void SceneManager::shutdown() {
    m_scenesNameMap.clear();
    m_scenesIndexMap.clear();
    m_scenes.clear();
}

void SceneManager::changeActiveScene(const String& sceneNameId) {
    FileSystem& fs = FileSystem::GetInstance();

    String filenameNoext = fs.join(sRootModelFolder, sceneNameId);

    json jsonObject;

    String filename = filenameNoext + ".json";
    if (fs.fileExists(filename)) {
        std::vector<byte> jsonData;

        fs.loadFileData(filename, &jsonData);
        if (json::accept(jsonData.begin(), jsonData.end())) {
            jsonObject = json::parse(jsonData.begin(), jsonData.end());
            LogInfo(sTag, "Loading scene: " + filename);
        } else {
            LogError(sTag, "Error loading model: " + sceneNameId);
        }
    }

    m_scenes.emplace_back(new Scene(jsonObject));

    Scene* scene = m_scenes.back().get();

    if (scene->load()) {
        const String& sceneName = scene->getName();
        const uint32 sceneIndex = sSceneIndex++;

        if (!sceneName.isEmpty()) {
            m_scenesNameMap[sceneName] = scene;
        }
        m_scenesIndexMap[sceneIndex] = scene;
    }

    LogInfo(sTag, "Scene '{}' successfully loaded"_format(filename));

    m_activeScene = scene;
}

Scene* SceneManager::getActiveScene() {
    return m_activeScene;
}

}  // namespace engine
