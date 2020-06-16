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
      : m_activeRenderer(nullptr),
        m_app(nullptr),
        m_logManager(nullptr),
        m_fileSystem(nullptr),
        m_sharedLibManager(nullptr),
        m_inputManager(nullptr),
        m_sceneManager(nullptr) {
    ENGINE_UNUSED(argc);
    ENGINE_UNUSED(argv);
    m_logManager = std::make_unique<LogManager>();
    m_fileSystem = std::make_unique<FileSystem>();
    m_sharedLibManager = std::make_unique<SharedLibManager>();
    m_inputManager = std::make_unique<InputManager>();
    m_sceneManager = std::make_unique<SceneManager>();
    m_asyncTaskRunner = std::make_unique<AsyncTaskRunner>();
}

Main::~Main() {
    shutdown();
    m_asyncTaskRunner.reset();
    m_sceneManager.reset();
    m_inputManager.reset();
    m_sharedLibManager.reset();
    m_fileSystem.reset();
    m_logManager.reset();
}

void Main::initialize(App* app) {
    setActiveRenderer();  // TODO: Change this to a configurable way

    if (!m_activeRenderer) {
        LogFatal(sTag, "Could not find an avaliable Renderer");
    }

    if (app != nullptr && m_app == nullptr) {
        LogInfo(sTag, "Initializing Engine");

        // 1. Initialize dependencies
        SDL_Init(0);

        // 2. Initialize the engine core subsystems
        LogManager::GetInstance().initialize();
        FileSystem::GetInstance().initialize();
        SharedLibManager::GetInstance().initialize();
        InputManager::GetInstance().initialize();

        // 3. Initialize plugins
        initializePlugins();

        // 4. Initialize the active Renderer
        if (!m_activeRenderer->initialize()) {
            LogFatal(sTag, "Could not initialize the Renderer");
        }

        // 5. Create the render window using the active renderer
        RenderWindow& window = m_activeRenderer->getRenderWindow();
        if (!window.create(app->getName(), app->getWindowSize())) {
            LogFatal(sTag, "Could not create the RenderWindow");
        }

        // 6. Initialize the renderer subsystems
        TextureManager::GetInstance().initialize();
        ShaderManager::GetInstance().initialize();
        ModelManager::GetInstance().initialize();
        SceneManager::GetInstance().initialize();

        // 7. Initialize the application
        m_app = app;
        m_app->initialize();
    }
}

void Main::run() {
    if (m_app == nullptr) {
        LogError(sTag, "The Engine has not being initialized");
        return;
    }

    RenderWindow& window = m_activeRenderer->getRenderWindow();

    Stopwatch timer;
    timer.start();
    while (!m_inputManager->exitRequested()) {
        m_app->m_deltaTime = timer.getElapsedTime();
        timer.restart();

        window.clear(Color::sBlack);

        m_app->update();

        Scene* active_scene = SceneManager::GetInstance().getActiveScene();
        if (active_scene) {
            active_scene->draw(window);
        }

        m_inputManager->advanceFrame();
        m_activeRenderer->advanceFrame();
    }
}

void Main::shutdown() {
    if (m_app != nullptr) {
        LogInfo(sTag, "Stopping Engine");

        // 1. Shutdown the application
        m_app->shutdown();
        m_app = nullptr;

        // 2. Shutdown the renderer subsystems
        SceneManager::GetInstance().shutdown();
        ModelManager::GetInstance().shutdown();
        TextureManager::GetInstance().shutdown();
        ShaderManager::GetInstance().shutdown();

        // 3. Shutdown the active Renderer
        // 4. Destroy the render window of the the active renderer
        m_activeRenderer->shutdown();
        m_activeRenderer = nullptr;
        m_renderers.clear();

        // 5. Shutdown plugins
        shutdownPlugins();

        // 6. Shutdown the engine core subsystems
        LogManager::GetInstance().shutdown();
        FileSystem::GetInstance().shutdown();
        SharedLibManager::GetInstance().shutdown();
        InputManager::GetInstance().shutdown();

        // 7. Shutdown dependencies
        SDL_Quit();
    }
}

void Main::setActiveScene(const String& scene_name) {
    ENGINE_UNUSED(scene_name);
}

void Main::loadPlugin(const String& pluginName) {
    // Load plugin library
    SharedLibrary* lib = m_sharedLibManager->load(pluginName);

    // Check for existence
    auto it = std::find(m_pluginLibs.begin(), m_pluginLibs.end(), lib);
    if (it == m_pluginLibs.end()) {
        m_pluginLibs.push_back(lib);

        // Call startup function
        PFN_START_PLUGIN pFunc = reinterpret_cast<PFN_START_PLUGIN>(lib->getSymbol("StartPlugin"));

        if (!pFunc) {
            LogFatal(sTag, "Cannot find symbol StartPlugin in library: " + pluginName);
        }

        // This must call InstallPlugin
        pFunc();
    }
}

void Main::unloadPlugin(const String& pluginName) {
    for (auto it = m_pluginLibs.begin(); it != m_pluginLibs.end(); it++) {
        SharedLibrary* shared_lib = *it;
        if (shared_lib->getName() == pluginName) {
            // Call plugin shutdown
            PFN_STOP_PLUGIN pFunc = (PFN_STOP_PLUGIN)shared_lib->getSymbol("StopPlugin");

            if (!pFunc) {
                const String& name = shared_lib->getName();
                LogFatal(sTag, "Cannot find symbol StopPlugin in library: " + name);
            }

            // This must call UninstallPlugin
            pFunc();

            // Unload library (destroyed by SharedLibManager)
            SharedLibManager::GetInstance().unload(shared_lib);
            it = m_pluginLibs.erase(it);
            return;
        }
    }
}

void Main::installPlugin(Plugin* plugin) {
    LogInfo(sTag, "Installing plugin: " + plugin->getName());

    m_plugins.push_back(plugin);
    plugin->install();

    // If the engine is already initialized, call the plugin init too
    if (m_app != nullptr) {
        plugin->initialize();
    }

    LogInfo(sTag, "Plugin successfully installed");
}

void Main::uninstallPlugin(Plugin* plugin) {
    LogInfo(sTag, "Uninstalling plugin: " + plugin->getName());

    auto it = std::find(m_plugins.begin(), m_plugins.end(), plugin);
    if (it != m_plugins.end()) {
        if (m_app != nullptr) {
            plugin->shutdown();
        }
        plugin->uninstall();
        m_plugins.erase(it);
    }

    LogInfo(sTag, "Plugin successfully uninstalled");
}

// TODO: Remove Renderer from Main
void Main::addRenderer(std::unique_ptr<Renderer>&& new_renderer) {
    m_renderers.push_back(std::move(new_renderer));
}

Renderer& Main::getActiveRenderer() {
    assert(getActiveRendererPtr());
    return *getActiveRendererPtr();
}

Renderer* Main::getActiveRendererPtr() {
    return m_activeRenderer;
}

void Main::executeAsync(AsyncTaskRunner::Task&& task) {
    m_asyncTaskRunner->execute(std::move(task));
}

void Main::initializePlugins() {
    for (auto& plugin : m_plugins) {
        plugin->initialize();
    }
}

void Main::shutdownPlugins() {
    // Unload all the Plugins loaded through shared libraries
    for (auto& plugin_lib : m_pluginLibs) {
        // Call plugin shutdown
        PFN_STOP_PLUGIN pFunc = reinterpret_cast<PFN_STOP_PLUGIN>(plugin_lib->getSymbol("StopPlugin"));

        if (!pFunc) {
            const String& name = plugin_lib->getName();
            LogFatal(sTag, "Cannot find symbol StopPlugin in library: " + name);
        }

        // This must call UninstallPlugin
        pFunc();

        // Unload library & destroy
        SharedLibManager::GetInstance().unload(plugin_lib);
    }
    m_pluginLibs.clear();

    // Now deal with any remaining plugins that were registered through
    // other means
    for (auto& plugin : m_plugins) {
        // Note this does NOT call uninstallPlugin - this shutdown is for
        // the detail objects
        plugin->uninstall();
    }
    m_plugins.clear();
}

void Main::setActiveRenderer() {
    // TODO: Add a configurable way to select this
    m_activeRenderer = !m_renderers.empty() ? m_renderers[0].get() : nullptr;
}

}  // namespace engine
