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

SharedLibrary::SharedLibrary(String name) : m_name(std::move(name)), m_handle(nullptr) {}

SharedLibrary::SharedLibrary(SharedLibrary&& other) noexcept {
    m_name = other.m_name;
    m_handle = other.m_handle;
    other.m_name.clear();
    other.m_handle = nullptr;
}

SharedLibrary::~SharedLibrary() {
    unload();
}

bool SharedLibrary::load() {
    // TODO: Add extension only if it does not exist
    if (m_handle != nullptr || m_name.isEmpty()) {
        return false;
    }

    String libName = LIBRARY_PREFIX + m_name + LIBRARY_EXTENSION;
#if PLATFORM_IS(PLATFORM_WINDOWS)
    auto wideString = libName.toWide();
    m_handle = LoadLibraryW(wideString.data());
#elif PLATFORM_IS(PLATFORM_LINUX | PLATFORM_MACOS | PLATFORM_IOS | PLATFORM_ANDROID)
    auto utf8string = libName.toUtf8();
    FileSystem& fs = FileSystem::GetInstance();
    String libExeDir = fs.join(fs.executableDirectory(), libName);
    if (fs.fileExists(libExeDir)) {
        utf8string = libExeDir.toUtf8();
    }
    m_handle = dlopen(utf8string.data(), RTLD_LAZY | RTLD_LOCAL);
#endif
    return (m_handle != nullptr);
}

void SharedLibrary::unload() {
    if (m_handle == nullptr) {
        return;
    }
#if PLATFORM_IS(PLATFORM_WINDOWS)
    FreeLibrary(reinterpret_cast<HMODULE>(m_handle));
#elif PLATFORM_IS(PLATFORM_LINUX | PLATFORM_MACOS | PLATFORM_IOS | PLATFORM_ANDROID)
    dlclose(m_handle);
#endif
    m_handle = nullptr;
}

String SharedLibrary::getErrorString() {
    if (m_name.isEmpty()) {
        return String("the library name must not be empty");
    }
#if PLATFORM_IS(PLATFORM_WINDOWS)
    LPWSTR lpMsgBuf;
    FormatMessageW((FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS), NULL,
                   GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);
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

const String& SharedLibrary::getName() const {
    return m_name;
}

void* SharedLibrary::getSymbol(const char* name) {
    if (m_handle == nullptr) {
        return nullptr;
    }
    void* address = nullptr;
#if PLATFORM_IS(PLATFORM_WINDOWS)
    address = GetProcAddress(reinterpret_cast<HMODULE>(m_handle), name);
#elif PLATFORM_IS(PLATFORM_LINUX | PLATFORM_MACOS | PLATFORM_IOS | PLATFORM_ANDROID)
    address = dlsym(m_handle, name);
#endif
    return address;
}

void* SharedLibrary::getSymbol(const String& name) {
    return getSymbol(name.getData());
}

}  // namespace engine
