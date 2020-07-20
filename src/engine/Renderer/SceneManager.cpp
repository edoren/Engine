#include <Renderer/SceneManager.hpp>

#include <Renderer/Model.hpp>
#include <Renderer/Scene.hpp>
#include <System/FileSystem.hpp>
#include <System/LogManager.hpp>
#include <System/StringView.hpp>
#include <Util/Container/Vector.hpp>

namespace engine {

namespace {

const StringView sTag("SceneManager");

const StringView sRootModelFolder("scenes");

uint32 sSceneIndex(0);

}  // namespace

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
        Vector<byte> jsonData;

        fs.loadFileData(filename, &jsonData);
        if (json::accept(jsonData.begin(), jsonData.end())) {
            jsonObject = json::parse(jsonData.begin(), jsonData.end());
            LogInfo(sTag, "Loading scene: {}", filename);
        } else {
            LogError(sTag, "Error loading model: {}", sceneNameId);
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

    LogInfo(sTag, "Scene '{}' successfully loaded", filename);

    m_activeScene = scene;
}

Scene* SceneManager::getActiveScene() {
    return m_activeScene;
}

}  // namespace engine
