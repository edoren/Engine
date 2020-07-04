#pragma once

#include <Util/Prerequisites.hpp>

#include <Core/Plugin.hpp>
#include <Core/SharedLibManager.hpp>
#include <Input/InputManager.hpp>
#include <Renderer/ModelManager.hpp>
#include <Renderer/Renderer.hpp>
#include <Renderer/SceneManager.hpp>
#include <System/FileSystem.hpp>
#include <System/LogManager.hpp>
#include <System/String.hpp>
#include <Util/Container/Vector.hpp>
#include <Util/Singleton.hpp>

#include <memory>

namespace engine {

class App;
class AsyncTaskRunner;

class ENGINE_API Main : public Singleton<Main> {
public:
    Main(int argc, char* argv[]);

    ~Main();

    void initialize(App* app);

    void run();

    void shutdown();

    void setActiveScene(const String& sceneName);

    void loadPlugin(const String& pluginName);

    void unloadPlugin(const String& pluginName);

    void installPlugin(Plugin* plugin);

    void uninstallPlugin(Plugin* plugin);

    void addRenderer(std::unique_ptr<Renderer>&& newRenderer);

    Renderer& getActiveRenderer();

    Renderer* getActiveRendererPtr();

    void executeAsync(Function<void()>&& task);

private:
    /**
     * @brief Initialize all the loaded installed
     */
    void initializePlugins();

    /**
     * @brief Shutdown all the loaded installed
     */
    void shutdownPlugins();

    /**
     * @brief Set the Renderer that the engine will use
     */
    void setActiveRenderer();

    Vector<SharedLibrary*> m_pluginLibs;
    Vector<Plugin*> m_plugins;

    Renderer* m_activeRenderer;
    Vector<std::unique_ptr<Renderer>> m_renderers;

    App* m_app;

    // Singletons
    std::unique_ptr<LogManager> m_logManager;
    std::unique_ptr<FileSystem> m_fileSystem;
    std::unique_ptr<SharedLibManager> m_sharedLibManager;
    std::unique_ptr<InputManager> m_inputManager;
    std::unique_ptr<SceneManager> m_sceneManager;
    std::unique_ptr<AsyncTaskRunner> m_asyncTaskRunner;
};

template<>
Main* Singleton<Main>::sInstance;

}  // namespace engine
