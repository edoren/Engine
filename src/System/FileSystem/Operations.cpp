// #include <System/FileSystem/Dependencies.hpp>
#include <System/FileSystem/Operations.hpp>
#include <Util/Prerequisites.hpp>

#include <SDL.h>

#if PLATFORM_IS(PLATFORM_WINDOWS)
#include <windows.h>
#elif PLATFORM_IS(PLATFORM_LINUX) || PLATFORM_IS(PLATFORM_ANDROID)
#include <unistd.h>
#endif

#define PATH_MAX_LENGTH 256

namespace engine {

namespace filesystem {

char8 GetSeparator() {
#if PLATFORM == PLATFORM_WINDOWS
    return '\\';
#else
    return '/';
#endif
}

String ExecutableDirectory() {
    String ret;
    char* path = SDL_GetBasePath();
    if (path) {
        ret = path;
        SDL_free(path);
    }
    return ret;
}

String CurrentWorkingDirectory() {
    String ret;
#if PLATFORM_IS(PLATFORM_WINDOWS)
    DWORD buffer_length = PATH_MAX_LENGTH;
    LPWSTR buffer = nullptr;
    while (true) {
        buffer = (LPWSTR)malloc(sizeof(WCHAR) * buffer_length + 1);
        DWORD num_characters = GetCurrentDirectoryW(buffer_length, buffer);
        if (num_characters > 0) {
            if (buffer[num_characters - 1] != L'\\') {
                buffer[num_characters++] = L'\\';
                buffer[num_characters] = L'\0';
            }
            ret = String::FromWide(buffer, buffer + num_characters);
            free(buffer);
            break;
        }
        free(buffer);
        buffer_length *= 2;
    }
#elif PLATFORM_IS(PLATFORM_LINUX) || PLATFORM_IS(PLATFORM_ANDROID)
    size_t buffer_length = PATH_MAX_LENGTH;
    char8* buffer = nullptr;
    while (true) {
        buffer = (char8*)malloc(sizeof(char8) * buffer_length + 1);
        char8* result = nullptr;
        result = getcwd(buffer, buffer_length);
        if (result != nullptr) {
            size_t num_characters = strlen(result);
            if (num_characters > 0 && buffer[num_characters - 1] != '/') {
                buffer[num_characters++] = '/';
                buffer[num_characters] = '\0';
            }
            ret = String::FromUtf8(buffer, buffer + num_characters);
            free(buffer);
            break;
        }
        free(buffer);
        buffer_length *= 2;
    }
#endif
    return ret;
}

String AbsolutePath(const String& path) {
    String ret;
#if PLATFORM_IS(PLATFORM_WINDOWS)
    auto wide_string = path.ToWide();
    DWORD buffer_length = PATH_MAX_LENGTH;
    LPWSTR buffer = nullptr;
    while (true) {
        buffer = (LPWSTR)malloc(sizeof(WCHAR) * buffer_length);
        DWORD num_characters = GetFullPathNameW(wide_string.data(),
                                                buffer_length, buffer, nullptr);
        if (num_characters > 0) {
            ret = String::FromWide(buffer, buffer + num_characters);
            free(buffer);
            break;
        }
        free(buffer);
        buffer_length *= 2;
    }
#elif PLATFORM_IS(PLATFORM_LINUX) || PLATFORM_IS(PLATFORM_ANDROID)
// TODO: NOT IMPLEMENTED
#endif
    return ret;
}

String AbsolutePath(const String& path, const String& base) {
    return AbsolutePath(Join(base, path));
}

String Join(const String& left, const String& right) {
    return left + GetSeparator() + right;
}

}  // namespace filesystem

}  // namespace engine
