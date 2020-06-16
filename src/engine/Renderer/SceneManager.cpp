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

    String filename_noext = fs.join(sRootModelFolder, sceneNameId);

    json json_object;

    String filename = filename_noext + ".json";
    if (fs.fileExists(filename)) {
        std::vector<byte> json_data;

        fs.loadFileData(filename, &json_data);
        if (json::accept(json_data.begin(), json_data.end())) {
            json_object = json::parse(json_data.begin(), json_data.end());
            LogInfo(sTag, "Loading scene: " + filename);
        } else {
            LogError(sTag, "Error loading model: " + sceneNameId);
        }
    }

    m_scenes.emplace_back(new Scene(json_object));

    Scene* scene = m_scenes.back().get();

    if (scene->load()) {
        const String& scene_name = scene->getName();
        const uint32 scene_index = sSceneIndex++;

        if (!scene_name.isEmpty()) {
            m_scenesNameMap[scene_name] = scene;
        }
        m_scenesIndexMap[scene_index] = scene;
    }

    LogInfo(sTag, "Scene '{}' successfully loaded"_format(filename));

    m_activeScene = scene;
}

Scene* SceneManager::getActiveScene() {
    return m_activeScene;
}

}  // namespace engine
