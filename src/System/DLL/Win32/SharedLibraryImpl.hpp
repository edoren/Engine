#pragma once

#include <Util/Precompiled.hpp>
#include <System/FileSystem.hpp>

#include <windows.h>

namespace engine {

namespace priv {

class SharedLibraryImpl {
public:
    explicit SharedLibraryImpl(const filesystem::Path& path);
    ~SharedLibraryImpl();

    void* LoadSymbol(const String& symbol);

private:
    HMODULE handle_;
};

}

}  // namespace engine
