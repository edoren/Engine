#pragma once

#include <Util/Prerequisites.hpp>

#include <Renderer/Scene.hpp>
#include <System/String.hpp>

namespace engine {

class ENGINE_API SceneManager : public Singleton<SceneManager> {
public:
    SceneManager();

    ~SceneManager();

    void initialize();

    void shutdown();

    void changeActiveScene(const String& scene_name);

    Scene* getActiveScene();

    static SceneManager& GetInstance();
    static SceneManager* GetInstancePtr();

private:
    Scene* m_active_scene;
    std::map<String, Scene*> m_scenes_name_map;
    std::map<uint32, Scene*> m_scenes_index_map;
    std::vector<std::unique_ptr<Scene>> m_scenes;
};

}  // namespace engine
