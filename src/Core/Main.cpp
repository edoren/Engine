#include <Core/App.hpp>
#include <Core/Main.hpp>
#include <Renderer/RenderWindow.hpp>
#include <Renderer/ShaderManager.hpp>
#include <System/Stopwatch.hpp>

#include <SDL.h>

namespace engine {

namespace {

const String sTag("Main");

}  // namespace

typedef void (*PFN_START_PLUGIN)(void);
typedef void (*PFN_STOP_PLUGIN)(void);

template <>
Main* Singleton<Main>::sInstance = nullptr;

Main& Main::GetInstance() {
    assert(sInstance);
    return (*sInstance);
}

Main* Main::GetInstancePtr() {
    return sInstance;
}

Main::Main(int argc, char* argv[])
      : m_is_initialized(false),
        m_active_renderer(nullptr),
        m_logger(nullptr),
        m_sharedlibs(nullptr),
        m_input(nullptr) {
    ENGINE_UNUSED(argc);
    ENGINE_UNUSED(argv);
    m_logger = new LogManager();
    m_file_system = new FileSystem();
    m_sharedlibs = new SharedLibManager();
    m_input = new InputManager();
}

Main::~Main() {
    Shutdown();
    delete m_input;
    delete m_sharedlibs;
    delete m_file_system;
    delete m_logger;
}

void Main::Initialize(App* app) {
    SetActiveRenderer();  // TODO: Change this to a configurable way

    if (!m_active_renderer) {
        LogFatal(sTag, "Could not find an avaliable Renderer");
    }

    if (!m_is_initialized && app != nullptr) {
        m_app = app;

        LogInfo(sTag, "Initializing Engine");

        // 1. Initialize dependencies
        SDL_Init(0);

        // 2. Initialize the engine core subsystems
        InputManager::GetInstance().Initialize();

        // 3. Initialize plugins
        InitializePlugins();

        // 4 Initialize the active Renderer
        if (!m_active_renderer->Initialize()) {
            LogFatal(sTag, "Could not initialize the Renderer");
        }

        // 5. Create the render window using the active renderer
        RenderWindow& window = m_active_renderer->GetRenderWindow();
        if (!window.Create(app->GetName(), app->GetWindowSize())) {
            LogFatal(sTag, "Could not create the RenderWindow");
        }

        // 6. Initialize the application
        m_app->Initialize();

        m_is_initialized = true;
    }
}

void Main::Run() {
    if (!m_is_initialized) {
        LogError(sTag, "The Engine has not being initialized");
        return;
    }

    RenderWindow& window = m_active_renderer->GetRenderWindow();

    Stopwatch timer;
    timer.Start();
    while (!m_input->exit_requested()) {
        m_app->m_delta_time = timer.GetElapsedTime();
        timer.Restart();

        window.Clear(Color::BLACK);

        m_app->Update();

        m_input->AdvanceFrame();
        m_active_renderer->AdvanceFrame();
    }
}

void Main::Shutdown() {
    if (m_is_initialized) {
        LogInfo(sTag, "Stopping Engine");

        m_app->Shutdown();

        InputManager::GetInstance().Shutdown();

        ShutdownPlugins();

        SDL_Quit();

        m_is_initialized = false;
    }
}

void Main::LoadPlugin(const String& name) {
    // Load plugin library
    SharedLibrary* lib = m_sharedlibs->Load(name);

    // Check for existence
    auto it = std::find(m_plugin_libs.begin(), m_plugin_libs.end(), lib);
    if (it == m_plugin_libs.end()) {
        m_plugin_libs.push_back(lib);

        // Call startup function
        PFN_START_PLUGIN pFunc =
            reinterpret_cast<PFN_START_PLUGIN>(lib->GetSymbol("StartPlugin"));

        if (!pFunc)
            LogFatal(sTag,
                     "Cannot find symbol StartPlugin in library: " + name);

        // This must call InstallPlugin
        pFunc();
    }
}

void Main::UnloadPlugin(const String& pluginName) {
    for (auto i = m_plugin_libs.begin(); i != m_plugin_libs.end(); i++) {
        if ((*i)->GetName() == pluginName) {
            // Call plugin shutdown
            PFN_STOP_PLUGIN pFunc =
                (PFN_STOP_PLUGIN)(*i)->GetSymbol("StopPlugin");

            if (!pFunc) {
                const String& name = (*i)->GetName();
                LogFatal(sTag,
                         "Cannot find symbol StopPlugin in library: " + name);
            }

            // This must call UninstallPlugin
            pFunc();

            // Unload library (destroyed by SharedLibManager)
            SharedLibManager::GetInstance().Unload(*i);
            m_plugin_libs.erase(i);
            return;
        }
    }
}

void Main::InstallPlugin(Plugin* plugin) {
    LogInfo(sTag, "Installing plugin: " + plugin->GetName());

    m_plugins.push_back(plugin);
    plugin->Install();

    // If the engine is already initialized, call the plugin init too
    if (m_is_initialized) {
        plugin->Initialize();
    }

    LogInfo(sTag, "Plugin successfully installed");
}

void Main::UninstallPlugin(Plugin* plugin) {
    LogInfo(sTag, "Uninstalling plugin: " + plugin->GetName());

    auto i = std::find(m_plugins.begin(), m_plugins.end(), plugin);
    if (i != m_plugins.end()) {
        if (m_is_initialized) plugin->Shutdown();
        plugin->Uninstall();
        m_plugins.erase(i);
    }

    LogInfo(sTag, "Plugin successfully uninstalled");
}

// TODO: Remove Renderer from Main
void Main::AddRenderer(Renderer* new_renderer) {
    m_renderers.push_back(new_renderer);
}

Renderer& Main::GetActiveRenderer() {
    assert(GetActiveRendererPtr());
    return *GetActiveRendererPtr();
}

Renderer* Main::GetActiveRendererPtr() {
    return m_active_renderer;
}

RendererFactory& Main::GetActiveRendererFactory() {
    return GetActiveRenderer().GetRendererFactory();
}

RendererFactory* Main::GetActiveRendererFactoryPtr() {
    return GetActiveRenderer().GetRendererFactoryPtr();
}

void Main::InitializePlugins() {
    for (auto i = m_plugins.begin(); i != m_plugins.end(); i++) {
        (*i)->Initialize();
    }
}

void Main::ShutdownPlugins() {
    // Unload all the Plugins loaded through shared libraries
    for (auto i = m_plugin_libs.rbegin(); i != m_plugin_libs.rend(); i++) {
        // Call plugin shutdown
        PFN_STOP_PLUGIN pFunc =
            reinterpret_cast<PFN_STOP_PLUGIN>((*i)->GetSymbol("StopPlugin"));

        if (!pFunc) {
            const String& name = (*i)->GetName();
            LogFatal(sTag, "Cannot find symbol StopPlugin in library: " + name);
        }

        // This must call UninstallPlugin
        pFunc();

        // Unload library & destroy
        SharedLibManager::GetInstance().Unload(*i);
    }
    m_plugin_libs.clear();

    // Now deal with any remaining plugins that were registered through other
    // means
    for (auto i = m_plugins.rbegin(); i != m_plugins.rend(); i++) {
        // Note this does NOT call uninstallPlugin - this shutdown is for the
        // detail objects
        (*i)->Uninstall();
    }
    m_plugins.clear();
}

void Main::SetActiveRenderer() {
    // TODO: Add a configurable way to select this
    m_active_renderer = m_renderers.size() ? m_renderers[0] : nullptr;
}

}  // namespace engine
