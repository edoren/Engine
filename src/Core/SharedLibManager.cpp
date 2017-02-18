#include <Core/SharedLibManager.hpp>
#include <System/LogManager.hpp>

namespace engine {

template <>
SharedLibManager* Singleton<SharedLibManager>::s_instance = nullptr;

SharedLibManager& SharedLibManager::GetInstance() {
    assert(s_instance);
    return (*s_instance);
}

SharedLibManager* SharedLibManager::GetInstancePtr() {
    return s_instance;
}

SharedLibManager::SharedLibManager() {}

SharedLibManager::~SharedLibManager() {
    // Unload & delete resources in turn
    for (auto it = m_libraries.begin(); it != m_libraries.end(); it++) {
        it->second.Unload();
    }
    m_libraries.clear();
}

SharedLibrary* SharedLibManager::Load(const String& name) {
    auto it = m_libraries.find(name);
    if (it != m_libraries.end()) {
        return &(it->second);
    } else {
        SharedLibrary lib(name);
        bool loaded = lib.Load();
        if (loaded) {
            auto result = m_libraries.emplace(name, std::move(lib));
            return result.second ? &(result.first->second) : nullptr;
        } else {
            LogError("SharedLibManager", lib.GetErrorString());
            LogFatal("SharedLibManager",
                     "Could not load SharedLibrary: " + name);
        }
    }

    return nullptr;
}

void SharedLibManager::Unload(SharedLibrary* lib) {
    lib->Unload();
    auto it = m_libraries.find(lib->GetName());
    if (it != m_libraries.end()) {
        m_libraries.erase(it);
    }
}

}  // namespace engine
