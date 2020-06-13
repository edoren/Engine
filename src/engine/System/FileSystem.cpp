#include <System/FileSystem.hpp>
#include <System/IOStream.hpp>
#include <System/LogManager.hpp>
#include <Util/Prerequisites.hpp>

#include <SDL2.h>

#if PLATFORM_IS(PLATFORM_WINDOWS)
    #include <windows.h>
#elif PLATFORM_IS(PLATFORM_LINUX | PLATFORM_MACOS | PLATFORM_IOS | PLATFORM_ANDROID)
    #include <unistd.h>
#endif

#define PATH_MAX_LENGTH 256

namespace engine {

namespace {

String sTag("FileSystem");

}  // namespace

template <>
FileSystem* Singleton<FileSystem>::sInstance = nullptr;

FileSystem& FileSystem::GetInstance() {
    return Singleton<FileSystem>::GetInstance();
}

FileSystem* FileSystem::GetInstancePtr() {
    return Singleton<FileSystem>::GetInstancePtr();
}

FileSystem::FileSystem() {
    m_search_paths = {
#if PLATFORM_IS(PLATFORM_ANDROID)
        ""
#else
        "data"
#endif
    };
}

FileSystem::~FileSystem() {}

void FileSystem::Initialize() {}

void FileSystem::Shutdown() {}

bool FileSystem::FileExists(const String& filename) const {
    IOStream file;
    for (const String& path : m_search_paths) {
        String file_path = Join(path, filename);
        if (file.Open(file_path, "r")) {
            return true;
        }
    }
    return false;
}

bool FileSystem::LoadFileData(const String& filename, String* dest) const {
    std::vector<byte> out;
    bool success = LoadFileData(filename, &out);
    if (success) {
        char8* begin = reinterpret_cast<char8*>(out.data());
        *dest = String::FromUtf8(begin, begin + out.size());
    }
    return success;
}

bool FileSystem::LoadFileData(const String& filename, std::vector<byte>* dest) const {
    IOStream file;

    auto filename_cpy = filename;
    filename_cpy.Replace('\\', GetOsSeparator());
    filename_cpy.Replace('/', GetOsSeparator());

    for (const String& path : m_search_paths) {
        String file_path = Join(path, filename_cpy);
        if (file.Open(file_path.GetData(), "rb")) break;
    }
    if (!file.IsOpen()) {
        LogError(sTag, "Error loading file: " + filename);
        return false;
    }

    std::size_t len = file.GetSize();
    dest->resize(len);
    std::size_t rlen = file.Read(dest->data(), 1, len);
    return len == rlen && len > 0;
}

char8 FileSystem::GetOsSeparator() const {
#if PLATFORM_IS(PLATFORM_WINDOWS)
    return '\\';
#else
    return '/';
#endif
}

String FileSystem::ExecutableDirectory() const {
    String ret;
    char* path = SDL_GetBasePath();
    if (path) {
        ret = path;
        SDL_free(path);
    }
    return ret;
}

String FileSystem::CurrentWorkingDirectory() const {
    String ret;
#if PLATFORM_IS(PLATFORM_WINDOWS)
    DWORD buffer_length = PATH_MAX_LENGTH;
    std::vector<std::remove_pointer_t<LPWSTR>> buffer;
    while (true) {
        buffer.resize(buffer_length + 1);
        DWORD num_characters = GetCurrentDirectoryW(buffer_length, buffer.data());
        if (num_characters > 0) {
            if (buffer[num_characters - 1] != L'\\') {
                buffer[num_characters++] = L'\\';
                buffer[num_characters] = L'\0';
            }
            ret = String::FromWide(buffer.data(), buffer.data() + num_characters);
            buffer.clear();
            break;
        }
        buffer.clear();
        buffer_length *= 2;
    }
#elif PLATFORM_IS(PLATFORM_LINUX | PLATFORM_MACOS | PLATFORM_IOS | PLATFORM_ANDROID)
    size_t buffer_length = PATH_MAX_LENGTH;
    std::vector<char8> buffer;
    while (true) {
        buffer.resize(buffer_length + 1);
        char8* result = nullptr;
        result = getcwd(buffer.data(), buffer_length);
        if (result != nullptr) {
            size_t num_characters = strlen(result);
            if (num_characters > 0 && buffer[num_characters - 1] != '/') {
                buffer[num_characters++] = '/';
                buffer[num_characters] = '\0';
            }
            ret = String::FromUtf8(buffer.data(), buffer.data() + num_characters);
            buffer.clear();
            break;
        }
        buffer.clear();
        buffer_length *= 2;
    }
#endif
    return ret;
}

String FileSystem::AbsolutePath(const String& /*path*/) const {
    String ret;

    return ret;
}

String FileSystem::NormalizePath(const String& path) const {
    bool is_absolute = IsAbsolutePath(path);
    std::vector<std::pair<const char8*, const char8*>> path_comps;

    auto AddPathComponent = [&path_comps, is_absolute](const char8* begin, const char8* end) {
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
                if (!is_absolute && (last.second - last.first) == 2 && std::memcmp(last.first, "..", 2) == 0) {
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
        if (*pathc_end == GetOsSeparator() && pathc_end > pathc_start) {
            AddPathComponent(pathc_start, pathc_end);
            pathc_start = pathc_end;
        }
        if (*pathc_start == GetOsSeparator()) pathc_start++;
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
        auto it = internal.cbegin();
        ret += String::FromUtf8(it, it + 2) + '\\';
#else
        ret += '/';
#endif
    }
    if (!is_absolute & path_comps.empty()) {
        ret += '.';
    } else {
        for (size_t i = 0; i < path_comps.size(); i++) {
            if (i) ret += GetOsSeparator();
            ret += String::FromUtf8(path_comps[i].first, path_comps[i].second);
        }
    }

#if PLATFORM_IS(PLATFORM_WINDOWS)
    // Fix separators on Windows
    ret.Replace('/', '\\');
#endif

    return ret;
}

bool FileSystem::IsAbsolutePath(const String& path) const {
    const auto& internal = path.ToUtf8();
    if (path.IsEmpty()) return false;
#if PLATFORM_IS(PLATFORM_WINDOWS)
    if (internal.size() > 2 && internal[1] == ':' && internal[2] == '\\') {
        return true;
    }
#elif PLATFORM_IS(PLATFORM_LINUX | PLATFORM_MACOS | PLATFORM_IOS | PLATFORM_ANDROID)
    if (internal[0] == '/') {
        return true;
    }
#endif
    return false;
}

String FileSystem::Join(const String& left, const String& right) const {
    if (right.IsEmpty()) return left;
    if (left.IsEmpty()) return right;

    if (IsAbsolutePath(right)) return right;

    String ret;
    const auto& internal = left.ToUtf8();
    char8 last_character = internal[internal.size() - 1];
    if (last_character == GetOsSeparator()) {
        ret = left + right;
    } else {
        ret = left + GetOsSeparator() + right;
    }
    return ret;
}

void FileSystem::SetSearchPaths(std::vector<String> search_paths) {
    m_search_paths = search_paths;
}

const std::vector<String>& FileSystem::GetSearchPaths() const {
    return m_search_paths;
}

void FileSystem::AddSearchPath(const String& path) {
    m_search_paths.push_back(path);
}

}  // namespace engine
