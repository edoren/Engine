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

SceneManager::SceneManager() {}

SceneManager::~SceneManager() {}

void SceneManager::Initialize() {}

void SceneManager::Shutdown() {}

void SceneManager::ChangeActiveScene(const String& scene_name_id) {
    FileSystem& fs = FileSystem::GetInstance();

    String filename_noext = fs.Join(sRootModelFolder, scene_name_id);

    json json_object;

    String filename = filename_noext + ".json";
    if (fs.FileExists(filename)) {
        std::vector<byte> json_data;

        fs.LoadFileData(filename, &json_data);
        if (json::accept(json_data.begin(), json_data.end())) {
            json_object = json::parse(json_data.begin(), json_data.end());
            LogInfo(sTag, "Loading scene: " + filename);
        } else {
            LogError(sTag, "Error loading model: " + scene_name_id);
        }
    }

    m_scenes.emplace_back(new Scene(json_object));

    Scene* scene = m_scenes.back().get();

    if (scene->Load()) {
        const String& scene_name = scene->GetName();
        const uint32 scene_index = sSceneIndex++;

        if (!scene_name.IsEmpty()) {
            m_scenes_name_map[scene_name] = scene;
        }
        m_scenes_index_map[scene_index] = scene;
    }

    m_active_scene = scene;
}

Scene* SceneManager::GetActiveScene() {
    return m_active_scene;
}

}  // namespace engine
