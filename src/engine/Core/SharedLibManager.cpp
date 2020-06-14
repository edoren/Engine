#include <Core/SharedLibManager.hpp>
#include <System/LogManager.hpp>

namespace engine {

namespace {

const String sTag("SharedLibManager");

}  // namespace

template <>
SharedLibManager* Singleton<SharedLibManager>::sInstance = nullptr;

SharedLibManager& SharedLibManager::GetInstance() {
    return Singleton<SharedLibManager>::GetInstance();
}

SharedLibManager* SharedLibManager::GetInstancePtr() {
    return Singleton<SharedLibManager>::GetInstancePtr();
}

SharedLibManager::SharedLibManager() {}

SharedLibManager::~SharedLibManager() {
    // Unload & delete resources in turn
    for (auto it = m_libraries.begin(); it != m_libraries.end(); it++) {
        it->second.unload();
    }
    m_libraries.clear();
}

void SharedLibManager::initialize() {}

void SharedLibManager::shutdown() {}

SharedLibrary* SharedLibManager::load(const String& name) {
    auto it = m_libraries.find(name);

    if (it != m_libraries.end()) {
        return &(it->second);
    }

    SharedLibrary lib(name);
    bool loaded = lib.load();
    if (loaded) {
        auto result = m_libraries.emplace(name, std::move(lib));
        return result.second ? &(result.first->second) : nullptr;
    }
    LogError(sTag, lib.getErrorString());
    LogFatal(sTag, "Could not load SharedLibrary: " + name);

    return nullptr;
}

void SharedLibManager::unload(SharedLibrary* lib) {
    lib->unload();
    auto it = m_libraries.find(lib->getName());
    if (it != m_libraries.end()) {
        m_libraries.erase(it);
    }
}

}  // namespace engine
