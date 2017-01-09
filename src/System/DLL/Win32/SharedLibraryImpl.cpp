#include "SharedLibraryImpl.hpp"

namespace engine {

namespace priv {

SharedLibraryImpl::SharedLibraryImpl(const filesystem::Path& path) : handle_(NULL) {
    filesystem::Path library(path);

    // if autocomplete extension flag
    library.ReplaceExtension(".dll");

    handle_ = LoadLibraryW(reinterpret_cast<LPCWSTR>(path.Str().ToUtf16().c_str()));
    if (!handle_) {
        FreeLibrary(handle_);
    }
}

SharedLibraryImpl::~SharedLibraryImpl() {
    FreeLibrary(handle_);
}

void* SharedLibraryImpl::LoadSymbol(const String &symbol) {
    FARPROC location = GetProcAddress(handle_, symbol.ToUtf8().c_str());
    return reinterpret_cast<void*>(location);
}

}  // namespace priv

}  // namespace engine
