#include <Core/App.hpp>
#include <Core/Main.hpp>
#include <Input/InputManager.hpp>
#include <Renderer/ModelManager.hpp>
#include <Renderer/RenderWindow.hpp>
#include <Renderer/Scene.hpp>
#include <Renderer/ShaderManager.hpp>
#include <Renderer/TextureManager.hpp>
#include <System/Stopwatch.hpp>

#include <Util/AsyncTaskRunner.hpp>

#include <SDL2.h>

namespace engine {

namespace {

const String sTag("Main");

}  // namespace

typedef void (*PFN_START_PLUGIN)(void);
typedef void (*PFN_STOP_PLUGIN)(void);

template <>
Main* Singleton<Main>::sInstance = nullptr;

Main& Main::GetInstance() {
    return Singleton<Main>::GetInstance();
}

Main* Main::GetInstancePtr() {
    return Singleton<Main>::GetInstancePtr();
}

Main::Main(int argc, char* argv[])
      : m_active_renderer(nullptr),

        m_app(nullptr),
        m_log_manager(nullptr),
        m_file_system(nullptr),
        m_shared_lib_manager(nullptr),
        m_input_manager(nullptr),
        m_scene_manager(nullptr) {
    ENGINE_UNUSED(argc);
    ENGINE_UNUSED(argv);
    m_log_manager = std::make_unique<LogManager>();
    m_file_system = std::make_unique<FileSystem>();
    m_shared_lib_manager = std::make_unique<SharedLibManager>();
    m_input_manager = std::make_unique<InputManager>();
    m_scene_manager = std::make_unique<SceneManager>();
    m_async_task_runner = std::make_unique<AsyncTaskRunner>();
}

Main::~Main() {
    Shutdown();
    m_async_task_runner.reset();
    m_scene_manager.reset();
    m_input_manager.reset();
    m_shared_lib_manager.reset();
    m_file_system.reset();
    m_log_manager.reset();
}

void Main::Initialize(App* app) {
    SetActiveRenderer();  // TODO: Change this to a configurable way

    if (!m_active_renderer) {
        LogFatal(sTag, "Could not find an avaliable Renderer");
    }

    if (app != nullptr && m_app == nullptr) {
        LogInfo(sTag, "Initializing Engine");

        // 1. Initialize dependencies
        SDL_Init(0);

        // 2. Initialize the engine core subsystems
        LogManager::GetInstance().Initialize();
        FileSystem::GetInstance().Initialize();
        SharedLibManager::GetInstance().Initialize();
        InputManager::GetInstance().Initialize();

        // 3. Initialize plugins
        InitializePlugins();

        // 4. Initialize the active Renderer
        if (!m_active_renderer->Initialize()) {
            LogFatal(sTag, "Could not initialize the Renderer");
        }

        // 5. Create the render window using the active renderer
        RenderWindow& window = m_active_renderer->GetRenderWindow();
        if (!window.Create(app->GetName(), app->GetWindowSize())) {
            LogFatal(sTag, "Could not create the RenderWindow");
        }

        // 6. Initialize the renderer subsystems
        TextureManager::GetInstance().Initialize();
        ShaderManager::GetInstance().Initialize();
        ModelManager::GetInstance().Initialize();
        SceneManager::GetInstance().Initialize();

        // 7. Initialize the application
        m_app = app;
        m_app->Initialize();
    }
}

void Main::Run() {
    if (m_app == nullptr) {
        LogError(sTag, "The Engine has not being initialized");
        return;
    }

    RenderWindow& window = m_active_renderer->GetRenderWindow();

    Stopwatch timer;
    timer.Start();
    while (!m_input_manager->exit_requested()) {
        m_app->m_delta_time = timer.GetElapsedTime();
        timer.Restart();

        window.Clear(Color::sBlack);

        m_app->Update();

        Scene* active_scene = SceneManager::GetInstance().GetActiveScene();
        if (active_scene) {
            active_scene->Draw(window);
        }

        m_input_manager->AdvanceFrame();
        m_active_renderer->AdvanceFrame();
    }
}

void Main::Shutdown() {
    if (m_app != nullptr) {
        LogInfo(sTag, "Stopping Engine");

        // 1. Shutdown the application
        m_app->Shutdown();
        m_app = nullptr;

        // 2. Shutdown the renderer subsystems
        SceneManager::GetInstance().Shutdown();
        ModelManager::GetInstance().Shutdown();
        TextureManager::GetInstance().Shutdown();
        ShaderManager::GetInstance().Shutdown();

        // 3. Shutdown the active Renderer
        // 4. Destroy the render window of the the active renderer
        m_active_renderer->Shutdown();
        m_active_renderer = nullptr;
        m_renderers.clear();

        // 5. Shutdown plugins
        ShutdownPlugins();

        // 6. Shutdown the engine core subsystems
        LogManager::GetInstance().Shutdown();
        FileSystem::GetInstance().Shutdown();
        SharedLibManager::GetInstance().Shutdown();
        InputManager::GetInstance().Shutdown();

        // 7. Shutdown dependencies
        SDL_Quit();
    }
}

void Main::SetActiveScene(const String& scene_name) {
    ENGINE_UNUSED(scene_name);
}

void Main::LoadPlugin(const String& name) {
    // Load plugin library
    SharedLibrary* lib = m_shared_lib_manager->Load(name);

    // Check for existence
    auto it = std::find(m_plugin_libs.begin(), m_plugin_libs.end(), lib);
    if (it == m_plugin_libs.end()) {
        m_plugin_libs.push_back(lib);

        // Call startup function
        PFN_START_PLUGIN pFunc = reinterpret_cast<PFN_START_PLUGIN>(lib->GetSymbol("StartPlugin"));

        if (!pFunc) {
            LogFatal(sTag, "Cannot find symbol StartPlugin in library: " + name);
        }

        // This must call InstallPlugin
        pFunc();
    }
}

void Main::UnloadPlugin(const String& pluginName) {
    for (auto it = m_plugin_libs.begin(); it != m_plugin_libs.end(); it++) {
        SharedLibrary* shared_lib = *it;
        if (shared_lib->GetName() == pluginName) {
            // Call plugin shutdown
            PFN_STOP_PLUGIN pFunc = (PFN_STOP_PLUGIN)shared_lib->GetSymbol("StopPlugin");

            if (!pFunc) {
                const String& name = shared_lib->GetName();
                LogFatal(sTag, "Cannot find symbol StopPlugin in library: " + name);
            }

            // This must call UninstallPlugin
            pFunc();

            // Unload library (destroyed by SharedLibManager)
            SharedLibManager::GetInstance().Unload(shared_lib);
            it = m_plugin_libs.erase(it);
            return;
        }
    }
}

void Main::InstallPlugin(Plugin* plugin) {
    LogInfo(sTag, "Installing plugin: " + plugin->GetName());

    m_plugins.push_back(plugin);
    plugin->Install();

    // If the engine is already initialized, call the plugin init too
    if (m_app != nullptr) {
        plugin->Initialize();
    }

    LogInfo(sTag, "Plugin successfully installed");
}

void Main::UninstallPlugin(Plugin* plugin) {
    LogInfo(sTag, "Uninstalling plugin: " + plugin->GetName());

    auto it = std::find(m_plugins.begin(), m_plugins.end(), plugin);
    if (it != m_plugins.end()) {
        if (m_app != nullptr) {
            plugin->Shutdown();
        }
        plugin->Uninstall();
        m_plugins.erase(it);
    }

    LogInfo(sTag, "Plugin successfully uninstalled");
}

// TODO: Remove Renderer from Main
void Main::AddRenderer(std::unique_ptr<Renderer>&& new_renderer) {
    m_renderers.push_back(std::move(new_renderer));
}

Renderer& Main::GetActiveRenderer() {
    assert(GetActiveRendererPtr());
    return *GetActiveRendererPtr();
}

Renderer* Main::GetActiveRendererPtr() {
    return m_active_renderer;
}

void Main::ExecuteAsync(AsyncTaskRunner::Task&& task) {
    m_async_task_runner->Execute(std::move(task));
}

void Main::InitializePlugins() {
    for (auto& plugin : m_plugins) {
        plugin->Initialize();
    }
}

void Main::ShutdownPlugins() {
    // Unload all the Plugins loaded through shared libraries
    for (auto& plugin_lib : m_plugin_libs) {
        // Call plugin shutdown
        PFN_STOP_PLUGIN pFunc = reinterpret_cast<PFN_STOP_PLUGIN>(plugin_lib->GetSymbol("StopPlugin"));

        if (!pFunc) {
            const String& name = plugin_lib->GetName();
            LogFatal(sTag, "Cannot find symbol StopPlugin in library: " + name);
        }

        // This must call UninstallPlugin
        pFunc();

        // Unload library & destroy
        SharedLibManager::GetInstance().Unload(plugin_lib);
    }
    m_plugin_libs.clear();

    // Now deal with any remaining plugins that were registered through
    // other means
    for (auto& plugin : m_plugins) {
        // Note this does NOT call uninstallPlugin - this shutdown is for
        // the detail objects
        plugin->Uninstall();
    }
    m_plugins.clear();
}

void Main::SetActiveRenderer() {
    // TODO: Add a configurable way to select this
    m_active_renderer = !m_renderers.empty() ? m_renderers[0].get() : nullptr;
}

}  // namespace engine
