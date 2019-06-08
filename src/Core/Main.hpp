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

    void Initialize(App* app);

    void Run();

    void Shutdown();

    void SetActiveScene(const String& scene_name);

    void LoadPlugin(const String& pluginName);

    void UnloadPlugin(const String& pluginName);

    void InstallPlugin(Plugin* plugin);

    void UninstallPlugin(Plugin* plugin);

    void AddRenderer(Renderer* new_renderer);

    Renderer& GetActiveRenderer();

    Renderer* GetActiveRendererPtr();

    RendererFactory& GetActiveRendererFactory();

    RendererFactory* GetActiveRendererFactoryPtr();

    void ExecuteAsync(Function<void()>&& task);

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
    void InitializePlugins();

    ////////////////////////////////////////////////////////////
    /// @brief Shutdown all the loaded installed
    ///
    ////////////////////////////////////////////////////////////
    void ShutdownPlugins();

    ////////////////////////////////////////////////////////////
    /// @brief Set the Renderer that the engine will use
    ///
    ////////////////////////////////////////////////////////////
    void SetActiveRenderer();

private:
    std::vector<SharedLibrary*> m_plugin_libs;
    std::vector<Plugin*> m_plugins;

    Renderer* m_active_renderer;
    std::vector<Renderer*> m_renderers;

    App* m_app;

    // Singletons
    LogManager* m_log_manager;
    FileSystem* m_file_system;
    SharedLibManager* m_shared_lib_manager;
    InputManager* m_input_manager;
    ModelManager* m_model_manager;
    SceneManager* m_scene_manager;
    AsyncTaskRunner* m_async_task_runner;
};

}  // namespace engine
