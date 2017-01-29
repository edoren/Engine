#pragma once

#include <Util/Precompiled.hpp>
#include <System/FileSystem.hpp>

namespace engine {

class ENGINE_API SharedLibrary : NonCopyable {
public:
    explicit SharedLibrary(const filesystem::Path& path);
    ~SharedLibrary();

    bool Load();
    void Unload();

    void* GetSymbol(const String& symbol);

private:
    filesystem::Path path_;
    void* handle_;
};

}  // namespace engine
