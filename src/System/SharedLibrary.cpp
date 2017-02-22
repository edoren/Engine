#include <System/SharedLibrary.hpp>

#if PLATFORM_IS(PLATFORM_WINDOWS)
#include <windows.h>
#define LIBRARY_PREFIX ""
#define LIBRARY_EXTENSION ".dll"
#elif PLATFORM_IS(PLATFORM_LINUX) || PLATFORM_IS(PLATFORM_ANDROID)
#include <dlfcn.h>
#define LIBRARY_PREFIX "lib"
#define LIBRARY_EXTENSION ".so"
#elif PLATFORM_IS(PLATFORM_MAC)
#include <dlfcn.h>
#define LIBRARY_PREFIX "lib"
#define LIBRARY_EXTENSION ".dylib"
#endif

namespace engine {

SharedLibrary::SharedLibrary(const String& name)
      : m_name(name), m_handle(nullptr) {}

SharedLibrary::SharedLibrary(SharedLibrary&& other) {
    m_name = other.m_name;
    m_handle = other.m_handle;
    other.m_name.Clear();
    other.m_handle = nullptr;
}

SharedLibrary::~SharedLibrary() {
    Unload();
}

bool SharedLibrary::Load() {
    // TODO: Add extension only if it does not exist
    if (m_handle != nullptr || m_name.IsEmpty()) return false;

    String lib_name = LIBRARY_PREFIX + m_name + LIBRARY_EXTENSION;
#if PLATFORM_IS(PLATFORM_WINDOWS)
    auto wide_string = lib_name.ToWide();
    m_handle = LoadLibraryW(wide_string.data());
#elif PLATFORM_IS(PLATFORM_LINUX) || PLATFORM_IS(PLATFORM_MAC)
    auto utf8string = lib_name.ToUtf8();
    m_handle = dlopen(utf8string.data(), RTLD_LAZY | RTLD_LOCAL);
#endif
    return (m_handle != nullptr);
}

void SharedLibrary::Unload() {
    if (m_handle == nullptr) return;
#if PLATFORM_IS(PLATFORM_WINDOWS)
    FreeLibrary(reinterpret_cast<HMODULE>(m_handle));
#elif PLATFORM_IS(PLATFORM_LINUX) || PLATFORM_IS(PLATFORM_MAC)
    dlclose(m_handle);
#endif
    m_handle = nullptr;
}

String SharedLibrary::GetErrorString() {
    if (m_name.IsEmpty()) return String("the library name must not be empty");
#if PLATFORM_IS(PLATFORM_WINDOWS)
    LPWSTR lpMsgBuf;
    FormatMessageW((FORMAT_MESSAGE_ALLOCATE_BUFFER |
                    FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS),
                   NULL, GetLastError(),
                   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf,
                   0, NULL);
    String ret = String::FromWide(lpMsgBuf, lpMsgBuf + std::wcslen(lpMsgBuf));
    LocalFree(lpMsgBuf);
    return ret;
#elif PLATFORM_IS(PLATFORM_LINUX) || PLATFORM_IS(PLATFORM_MAC)
    return String(dlerror());
#else
    return String("");
#endif
}

const String& SharedLibrary::GetName() const {
    return m_name;
}

void* SharedLibrary::GetSymbol(const String& symbol) {
    if (m_handle == nullptr) return nullptr;
    void* address = nullptr;
#if PLATFORM_IS(PLATFORM_WINDOWS)
    address =
        GetProcAddress(reinterpret_cast<HMODULE>(m_handle), symbol.GetData());
#elif PLATFORM_IS(PLATFORM_LINUX) || PLATFORM_IS(PLATFORM_MAC)
    address = dlsym(m_handle, symbol.GetData());
#endif
    return address;
}

}  // namespace engine
