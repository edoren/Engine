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

    void setActiveScene(const String& scene_name);

    void loadPlugin(const String& pluginName);

    void unloadPlugin(const String& pluginName);

    void installPlugin(Plugin* plugin);

    void uninstallPlugin(Plugin* plugin);

    void addRenderer(std::unique_ptr<Renderer>&& new_renderer);

    Renderer& getActiveRenderer();

    Renderer* getActiveRendererPtr();

    void executeAsync(Function<void()>&& task);

    ////////////////////////////////////////////////////////////
    /// @brief Override standard Singleton retrieval.
    ///
    /// @remarks Why do we do this? Well, it's because the Singleton
    ///          implementation is in a .hpp file, which means it gets
    ///          compiled into anybody who includes it. This is needed
    ///          for the Singleton template to work, but we actually
    ///          only compiled into the implementation of the class
    ///          based on the Singleton, not all of them. If we don't
    ///          change this, we get link errors when trying to use the
    ///          Singleton-based class from an outside dll.
    ///
    /// @par This method just delegates to the template version anyway,
    ///      but the implementation stays in this single compilation unit,
    ///      preventing link errors.
    ////////////////////////////////////////////////////////////
    static Main& GetInstance();

    ////////////////////////////////////////////////////////////
    /// @brief Override standard Singleton retrieval.
    ///
    /// @remarks Why do we do this? Well, it's because the Singleton
    ///          implementation is in a .hpp file, which means it gets
    ///          compiled into anybody who includes it. This is needed
    ///          for the Singleton template to work, but we actually
    ///          only compiled into the implementation of the class
    ///          based on the Singleton, not all of them. If we don't
    ///          change this, we get link errors when trying to use the
    ///          Singleton-based class from an outside dll.
    ///
    /// @par This method just delegates to the template version anyway,
    ///      but the implementation stays in this single compilation unit,
    ///      preventing link errors.
    ////////////////////////////////////////////////////////////
    static Main* GetInstancePtr();

private:
    ////////////////////////////////////////////////////////////
    /// @brief Initialize all the loaded installed
    ///
    ////////////////////////////////////////////////////////////
    void initializePlugins();

    ////////////////////////////////////////////////////////////
    /// @brief Shutdown all the loaded installed
    ///
    ////////////////////////////////////////////////////////////
    void shutdownPlugins();

    ////////////////////////////////////////////////////////////
    /// @brief Set the Renderer that the engine will use
    ///
    ////////////////////////////////////////////////////////////
    void setActiveRenderer();

    std::vector<SharedLibrary*> m_plugin_libs;
    std::vector<Plugin*> m_plugins;

    Renderer* m_active_renderer;
    std::vector<std::unique_ptr<Renderer>> m_renderers;

    App* m_app;

    // Singletons
    std::unique_ptr<LogManager> m_log_manager;
    std::unique_ptr<FileSystem> m_file_system;
    std::unique_ptr<SharedLibManager> m_shared_lib_manager;
    std::unique_ptr<InputManager> m_input_manager;
    std::unique_ptr<SceneManager> m_scene_manager;
    std::unique_ptr<AsyncTaskRunner> m_async_task_runner;
};

}  // namespace engine
