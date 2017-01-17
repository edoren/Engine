#include <System/SharedLibrary.hpp>

#if PLATFORM_IS(PLATFORM_WINDOWS)
#include <windows.h>
#elif PLATFORM_IS(PLATFORM_LINUX)
#elif PLATFORM_IS(PLATFORM_MAC)
#endif

namespace engine {

SharedLibrary::SharedLibrary(const filesystem::Path& path)
      : path_(path), handle_(nullptr) {}

SharedLibrary::~SharedLibrary() {
    Unload();
}

void SharedLibrary::Load() {
#if PLATFORM_IS(PLATFORM_WINDOWS)
    filesystem::Path library(path_);

    // if autocomplete extension flag
    library.ReplaceExtension(".dll");

    std::u16string utf16string = path_.Str().ToUtf16();
    handle_ = LoadLibraryW(reinterpret_cast<LPCWSTR>(utf16string.c_str()));
    if (!handle_) {
        FreeLibrary(reinterpret_cast<HMODULE>(handle_));
    }
#elif PLATFORM_IS(PLATFORM_LINUX)
#elif PLATFORM_IS(PLATFORM_MAC)
#endif
}

void SharedLibrary::Unload() {
#if PLATFORM_IS(PLATFORM_WINDOWS)
    FreeLibrary(reinterpret_cast<HMODULE>(handle_));
#elif PLATFORM_IS(PLATFORM_LINUX)
#elif PLATFORM_IS(PLATFORM_MAC)
#endif
}

void* SharedLibrary::GetSymbol(const String& symbol) {
    void* address = nullptr;
#if PLATFORM_IS(PLATFORM_WINDOWS)
    address = GetProcAddress(reinterpret_cast<HMODULE>(handle_),
                             symbol.ToUtf8().c_str());
#elif PLATFORM_IS(PLATFORM_LINUX)
#elif PLATFORM_IS(PLATFORM_MAC)
#endif
    return address;
}

}  // namespace engine
