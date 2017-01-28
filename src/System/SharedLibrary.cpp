#include <System/SharedLibrary.hpp>

#if PLATFORM_IS(PLATFORM_WINDOWS)
#include <windows.h>
#define LIBRARY_EXTENSION ".dll"
#elif PLATFORM_IS(PLATFORM_LINUX)
#include <dlfcn.h>
#define LIBRARY_EXTENSION ".so"
#elif PLATFORM_IS(PLATFORM_MAC)
#include <dlfcn.h>
#define LIBRARY_EXTENSION ".dylib"
#endif

namespace engine {

SharedLibrary::SharedLibrary(const filesystem::Path& path)
      : path_(path), handle_(nullptr) {}

SharedLibrary::~SharedLibrary() {
    Unload();
}

bool SharedLibrary::Load() {
    filesystem::Path library(path_);
    // TODO: Add extension only if it does not exist
    library.ReplaceExtension(LIBRARY_EXTENSION);
#if PLATFORM_IS(PLATFORM_WINDOWS)
    std::u16string utf16string = library.Str().ToUtf16();
    handle_ = LoadLibraryW(reinterpret_cast<LPCWSTR>(utf16string.c_str()));
    if (handle_ == nullptr) {
        // TODO: Handle errors windows
        // DWORD error_code = GetLastError();
    }
#elif PLATFORM_IS(PLATFORM_LINUX) || PLATFORM_IS(PLATFORM_MAC)
    std::string utf8string = library.Str().ToUtf8();
    handle = dlopen(utf8string.c_str(), RTLD_LAZY | RTLD_LOCAL);
    if (handle == nullptr) {
        // TODO: Handle errors posix
        // const char* error = dlerror();
    }
#endif
    return (handle_ != nullptr);
}

void SharedLibrary::Unload() {
    if (handle_ != nullptr) {
#if PLATFORM_IS(PLATFORM_WINDOWS)
        FreeLibrary(reinterpret_cast<HMODULE>(handle_));
#elif PLATFORM_IS(PLATFORM_LINUX) || PLATFORM_IS(PLATFORM_MAC)
        dlclose(handle_);
#endif
    }
}

void* SharedLibrary::GetSymbol(const String& symbol) {
    void* address = nullptr;
    if (handle_ != nullptr) {
#if PLATFORM_IS(PLATFORM_WINDOWS)
        address = GetProcAddress(reinterpret_cast<HMODULE>(handle_),
                                 symbol.ToUtf8().c_str());
#elif PLATFORM_IS(PLATFORM_LINUX) || PLATFORM_IS(PLATFORM_MAC)
        address = dlsym(handle_, symbol.ToUtf8().c_str());
#endif
    }
    return address;
}

}  // namespace engine
