#pragma once

#include <Util/Prerequisites.hpp>

#include <Renderer/Scene.hpp>
#include <System/String.hpp>
#include <Util/Container/Vector.hpp>
#include <Util/Singleton.hpp>

#include <map>
#include <memory>

namespace engine {

class ENGINE_API SceneManager : public Singleton<SceneManager> {
public:
    SceneManager();

    ~SceneManager();

    void initialize();

    void shutdown();

    void changeActiveScene(const String& sceneName);

    Scene* getActiveScene();

private:
    Scene* m_activeScene;
    std::map<String, Scene*> m_scenesNameMap;
    std::map<uint32, Scene*> m_scenesIndexMap;
    Vector<std::unique_ptr<Scene>> m_scenes;
};

}  // namespace engine
