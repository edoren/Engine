#include <Core/Main.hpp>

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
        m_input(nullptr),
        m_resources(nullptr) {
    ENGINE_UNUSED(argc);
    ENGINE_UNUSED(argv);
    m_logger = new LogManager();
    m_file_system = new FileSystem();
    m_sharedlibs = new SharedLibManager();
    m_input = new InputManager();
    m_resources = new ResourceManager();
}

Main::~Main() {
    Shutdown();
    delete m_resources;
    delete m_input;
    delete m_sharedlibs;
    delete m_file_system;
    delete m_logger;
}

void Main::Initialize() {
    SetActiveRenderer();  // TODO: Change this to a configurable way

    if (!m_active_renderer) {
        LogFatal(sTag, "Could not find an avaliable Renderer");
    }

    if (!m_is_initialized) {
        LogInfo(sTag, "Initializing Engine");

        SDL_Init(0);

        InputManager::GetInstance().Initialize();
        ResourceManager::GetInstance().Initialize();

        bool status = GetActiveRenderer().Initialize();
        if (!status) {
            LogFatal(sTag, "Could not initialize the Renderer");
        }

        InitializePlugins();

        m_is_initialized = true;
    }
}

void Main::Shutdown() {
    if (m_is_initialized) {
        LogInfo(sTag, "Stopping Engine");

        InputManager::GetInstance().Shutdown();
        ResourceManager::GetInstance().Shutdown();

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
            LogFatal(sTag,
                     "Cannot find symbol StopPlugin in library: " + name);
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
