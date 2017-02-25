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
#if PLATFORM_IS(PLATFORM_WINDOWS)
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

String AbsolutePath(const String& /*path*/) {
    String ret;

    return ret;
}

String NormalizePath(const String& path) {
    bool is_absolute = IsAbsolute(path);
    std::vector<std::pair<const char8*, const char8*>> path_comps;

    auto AddPathComponent = [&path_comps, is_absolute](const char8* begin,
                                                       const char8* end) {
        size_t seq_size = end - begin;

        // Ignore the component if the . directories
        if (seq_size == 1 && std::memcmp(begin, ".", 1) == 0) {
            return;
        }

        // If the component is a .. directory
        if (seq_size == 2 && std::memcmp(begin, "..", 2) == 0) {
            // Check if the path_comps is empty
            if (!path_comps.empty()) {
                // If the last element is a .. directory, append another one
                // if not just pop_back the last component
                auto& last = path_comps.back();
                if (!is_absolute && (last.second - last.first) == 2 &&
                    std::memcmp(last.first, "..", 2) == 0) {
                    path_comps.emplace_back(begin, end);
                } else {
                    path_comps.pop_back();
                }
            }
            // Only add .. directories if the path is not absolute
            else if (!is_absolute) {
                path_comps.emplace_back(begin, end);
            }
            return;
        }

        // Add the path component
        path_comps.emplace_back(begin, end);
    };

    const auto& internal = path.ToUtf8();

    // Get the path component without the drive on Windows
    size_t begin_offset = 0;
#if PLATFORM_IS(PLATFORM_WINDOWS)
    if (is_absolute) {
        begin_offset = 2;
    }
#endif

    // Split the string by the separator
    const char8* pathc_start = internal.data() + begin_offset;
    const char8* pathc_end = pathc_start;
    while (*pathc_end != 0) {
        // Get the path component from the start and end iterators
        if (*pathc_end == GetSeparator() && pathc_end > pathc_start) {
            AddPathComponent(pathc_start, pathc_end);
            pathc_start = pathc_end;
        }
        if (*pathc_start == GetSeparator()) pathc_start++;
        pathc_end++;
    }
    // Get the last path component
    // (if the string does not finish in a separator)
    if (pathc_start != pathc_end) {
        AddPathComponent(pathc_start, pathc_end);
    }

    // Create the result normalized path
    String ret;
    if (is_absolute) {
#if PLATFORM_IS(PLATFORM_WINDOWS)
        ret += String::FromUtf8(internal.cbegin(), internal.cbegin() + 2);
        ret += "\\";
#else
        ret += "/";
#endif
    }
    if (!is_absolute & path_comps.empty()) {
        ret += ".";
    } else {
        for (size_t i = 0; i < path_comps.size(); i++) {
            if (i) ret += GetSeparator();
            ret += String::FromUtf8(path_comps[i].first, path_comps[i].second);
        }
    }

    return ret;
}

bool IsAbsolute(const String& path) {
    const auto& internal = path.ToUtf8();
    if (path.IsEmpty()) return false;
#if PLATFORM_IS(PLATFORM_WINDOWS)
    if (internal.size() > 2 && internal[1] == ':' && internal[2] == '\\') {
        return true;
    }
#elif PLATFORM_IS(PLATFORM_LINUX) || PLATFORM_IS(PLATFORM_ANDROID)
    if (internal[0] == '/') {
        return true;
    }
#endif
    return false;
}

String Join(const String& left, const String& right) {
    if (right.IsEmpty()) return left;
    if (left.IsEmpty()) return right;

    if (IsAbsolute(right)) return right;

    String ret;
    const auto& internal = left.ToUtf8();
    char8 last_character = internal[internal.size() - 1];
    if (last_character == GetSeparator()) {
        ret = left + right;
    } else {
        ret = left + GetSeparator() + right;
    }
    return ret;
}

}  // namespace filesystem

}  // namespace engine
