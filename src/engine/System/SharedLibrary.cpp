#include <System/SharedLibrary.hpp>

#include <System/FileSystem.hpp>

#if PLATFORM_IS(PLATFORM_WINDOWS)
#include <windows.h>
#define LIBRARY_PREFIX ""
#define LIBRARY_EXTENSION ".dll"
#elif PLATFORM_IS(PLATFORM_LINUX | PLATFORM_ANDROID)
#include <dlfcn.h>
#define LIBRARY_PREFIX "lib"
#define LIBRARY_EXTENSION ".so"
#elif PLATFORM_IS(PLATFORM_MACOS | PLATFORM_IOS)
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
#elif PLATFORM_IS(PLATFORM_LINUX | PLATFORM_MACOS | PLATFORM_IOS | PLATFORM_ANDROID)
    auto utf8string = lib_name.ToUtf8();
    FileSystem& fs = FileSystem::GetInstance();
    String lib_exe_dir = fs.Join(fs.ExecutableDirectory(), lib_name);
    if (fs.FileExists(lib_exe_dir)) {
        utf8string = lib_exe_dir.ToUtf8();
    }
    m_handle = dlopen(utf8string.data(), RTLD_LAZY | RTLD_LOCAL);
#endif
    return (m_handle != nullptr);
}

void SharedLibrary::Unload() {
    if (m_handle == nullptr) return;
#if PLATFORM_IS(PLATFORM_WINDOWS)
    FreeLibrary(reinterpret_cast<HMODULE>(m_handle));
#elif PLATFORM_IS(PLATFORM_LINUX | PLATFORM_MACOS | PLATFORM_IOS | PLATFORM_ANDROID)
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
    std::size_t msg_len = std::wcslen(lpMsgBuf);
    if (msg_len >= 2) msg_len -= 2;  // Remove the /r/n characters
    String ret = String::FromWide(lpMsgBuf, lpMsgBuf + msg_len);
    LocalFree(lpMsgBuf);
    return ret;
#elif PLATFORM_IS(PLATFORM_LINUX | PLATFORM_MACOS | PLATFORM_IOS | PLATFORM_ANDROID)
    return String(dlerror());
#else
    return String("");
#endif
}

const String& SharedLibrary::GetName() const {
    return m_name;
}

void* SharedLibrary::GetSymbol(const char* name) {
    if (m_handle == nullptr) return nullptr;
    void* address = nullptr;
#if PLATFORM_IS(PLATFORM_WINDOWS)
    address = GetProcAddress(reinterpret_cast<HMODULE>(m_handle), name);
#elif PLATFORM_IS(PLATFORM_LINUX | PLATFORM_MACOS | PLATFORM_IOS | PLATFORM_ANDROID)
    address = dlsym(m_handle, name);
#endif
    return address;
}

void* SharedLibrary::GetSymbol(const String& name) {
    return GetSymbol(name.GetData());
}

}  // namespace engine
