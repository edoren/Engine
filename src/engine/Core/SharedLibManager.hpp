#pragma once

#include <Util/Prerequisites.hpp>

#include <System/SharedLibrary.hpp>
#include <System/String.hpp>
#include <Util/Singleton.hpp>

#include <map>

namespace engine {

class ENGINE_API SharedLibManager : public Singleton<SharedLibManager> {
public:
    SharedLibManager();

    ~SharedLibManager();

    void initialize();

    void shutdown();

    SharedLibrary* load(const StringView& name);

    void unload(SharedLibrary* lib);

private:
    std::map<String, SharedLibrary> m_libraries;
};

}  // namespace engine
