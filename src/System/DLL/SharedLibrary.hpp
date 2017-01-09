#pragma once

#include <Util/Precompiled.hpp>
#include <System/FileSystem.hpp>

#if PLATFORM_IS(PLATFORM_WINDOWS)
#include "Win32/SharedLibraryImpl.hpp"
#elif PLATFORM_IS(PLATFORM_LINUX)
#elif PLATFORM_IS(PLATFORM_MAC)
#endif

namespace engine {

class SharedLibrary : NonCopyable {
public:
    explicit SharedLibrary(const filesystem::Path& path);
    ~SharedLibrary();

    template<typename T>
    T* LoadSymbol(const String& symbol) {
        return reinterpret_cast<T*>(impl_->LoadSymbol(symbol));
    }

private:
    priv::SharedLibraryImpl* impl_;
};

}  // namespace engine
