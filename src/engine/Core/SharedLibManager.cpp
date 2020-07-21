#include <Core/SharedLibManager.hpp>

#include <System/LogManager.hpp>
#include <System/StringView.hpp>

namespace engine {

namespace {

const StringView sTag("SharedLibManager");

}  // namespace

SharedLibManager::SharedLibManager() = default;

SharedLibManager::~SharedLibManager() {
    // Unload & delete resources in turn
    for (auto& library : m_libraries) {
        library.second.unload();
    }
    m_libraries.clear();
}

void SharedLibManager::initialize() {}

void SharedLibManager::shutdown() {}

SharedLibrary* SharedLibManager::load(const StringView& name) {
    auto it = std::find_if(m_libraries.begin(), m_libraries.end(), [&name](auto& pair) {
        return pair.first == name;
    });

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
    LogFatal(sTag, "Could not load SharedLibrary: {}", name);

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
