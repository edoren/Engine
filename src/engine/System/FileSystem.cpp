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
    m_searchPaths = {
#if PLATFORM_IS(PLATFORM_ANDROID)
        ""
#else
        "data"
#endif
    };
}

FileSystem::~FileSystem() = default;

void FileSystem::initialize() {}

void FileSystem::shutdown() {}

bool FileSystem::fileExists(const String& filename) const {
    IOStream file;
    for (const String& path : m_searchPaths) {
        String filePath = join(path, filename);
        if (file.open(filePath, "r")) {
            return true;
        }
    }
    return false;
}

bool FileSystem::loadFileData(const String& filename, String* dest) const {
    std::vector<byte> out;
    bool success = loadFileData(filename, &out);
    if (success) {
        auto* begin = reinterpret_cast<char8*>(out.data());
        *dest = String::FromUtf8(begin, begin + out.size());
    }
    return success;
}

bool FileSystem::loadFileData(const String& filename, std::vector<byte>* dest) const {
    IOStream file;

    auto filenameCpy = filename;
    filenameCpy.replace('\\', getOsSeparator());
    filenameCpy.replace('/', getOsSeparator());

    for (const String& path : m_searchPaths) {
        String filePath = join(path, filenameCpy);
        if (file.open(filePath.getData(), "rb")) {
            break;
        }
    }
    if (!file.isOpen()) {
        LogError(sTag, "Error loading file: " + filename);
        return false;
    }

    std::size_t len = file.getSize();
    dest->resize(len);
    std::size_t rlen = file.read(dest->data(), 1, len);
    return len == rlen && len > 0;
}

char8 FileSystem::getOsSeparator() const {
#if PLATFORM_IS(PLATFORM_WINDOWS)
    return '\\';
#else
    return '/';
#endif
}

String FileSystem::executableDirectory() const {
    String ret;
    char* path = SDL_GetBasePath();
    if (path) {
        ret = path;
        SDL_free(path);
    }
    return ret;
}

String FileSystem::currentWorkingDirectory() const {
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
    size_t bufferLength = PATH_MAX_LENGTH;
    std::vector<char8> buffer;
    while (true) {
        buffer.resize(bufferLength + 1);
        char8* result = nullptr;
        result = getcwd(buffer.data(), bufferLength);
        if (result != nullptr) {
            size_t numCharacters = strlen(result);
            if (numCharacters > 0 && buffer[numCharacters - 1] != '/') {
                buffer[numCharacters++] = '/';
                buffer[numCharacters] = '\0';
            }
            ret = String::FromUtf8(buffer.data(), buffer.data() + numCharacters);
            buffer.clear();
            break;
        }
        buffer.clear();
        bufferLength *= 2;
    }
#endif
    return ret;
}

String FileSystem::absolutePath(const String& /*path*/) const {
    String ret;

    return ret;
}

String FileSystem::normalizePath(const String& path) const {
    bool isAbsolute = isAbsolutePath(path);
    std::vector<std::pair<const char8*, const char8*>> pathComps;

    auto addPathComponent = [&pathComps, isAbsolute](const char8* begin, const char8* end) {
        size_t seqSize = end - begin;

        // Ignore the component if the . directories
        if (seqSize == 1 && std::memcmp(begin, ".", 1) == 0) {
            return;
        }

        // If the component is a .. directory
        if (seqSize == 2 && std::memcmp(begin, "..", 2) == 0) {
            // Check if the path_comps is empty
            if (!pathComps.empty()) {
                // If the last element is a .. directory, append another one
                // if not just pop_back the last component
                auto& last = pathComps.back();
                if (!isAbsolute && (last.second - last.first) == 2 && std::memcmp(last.first, "..", 2) == 0) {
                    pathComps.emplace_back(begin, end);
                } else {
                    pathComps.pop_back();
                }
            }
            // Only add .. directories if the path is not absolute
            else if (!isAbsolute) {
                pathComps.emplace_back(begin, end);
            }
            return;
        }

        // Add the path component
        pathComps.emplace_back(begin, end);
    };

    const auto& internal = path.toUtf8();

    // Get the path component without the drive on Windows
    size_t beginOffset = 0;
#if PLATFORM_IS(PLATFORM_WINDOWS)
    if (is_absolute) {
        begin_offset = 2;
    }
#endif

    // Split the string by the separator
    const char8* pathcStart = internal.data() + beginOffset;
    const char8* pathcEnd = pathcStart;
    while (*pathcEnd != 0) {
        // Get the path component from the start and end iterators
        if (*pathcEnd == getOsSeparator() && pathcEnd > pathcStart) {
            addPathComponent(pathcStart, pathcEnd);
            pathcStart = pathcEnd;
        }
        if (*pathcStart == getOsSeparator()) {
            pathcStart++;
        }
        pathcEnd++;
    }
    // Get the last path component
    // (if the string does not finish in a separator)
    if (pathcStart != pathcEnd) {
        addPathComponent(pathcStart, pathcEnd);
    }

    // Create the result normalized path
    String ret;
    if (isAbsolute) {
#if PLATFORM_IS(PLATFORM_WINDOWS)
        auto it = internal.cbegin();
        ret += String::FromUtf8(it, it + 2) + '\\';
#else
        ret += '/';
#endif
    }
    if (!isAbsolute & pathComps.empty()) {
        ret += '.';
    } else {
        for (size_t i = 0; i < pathComps.size(); i++) {
            if (i) {
                ret += getOsSeparator();
            }
            ret += String::FromUtf8(pathComps[i].first, pathComps[i].second);
        }
    }

#if PLATFORM_IS(PLATFORM_WINDOWS)
    // Fix separators on Windows
    ret.replace('/', '\\');
#endif

    return ret;
}

bool FileSystem::isAbsolutePath(const String& path) const {
    const auto& internal = path.toUtf8();
    if (path.isEmpty()) {
        return false;
    }
#if PLATFORM_IS(PLATFORM_WINDOWS)
    return internal.size() > 2 && internal[1] == ':' && internal[2] == '\\';
#elif PLATFORM_IS(PLATFORM_LINUX | PLATFORM_MACOS | PLATFORM_IOS | PLATFORM_ANDROID)
    return internal[0] == '/';
#else
    return false;
#endif
}

String FileSystem::join(const String& left, const String& right) const {
    if (right.isEmpty()) {
        return left;
    }
    if (left.isEmpty()) {
        return right;
    }

    if (isAbsolutePath(right)) {
        return right;
    }

    String ret;
    const auto& internal = left.toUtf8();
    char8 lastCharacter = internal[internal.size() - 1];
    if (lastCharacter == getOsSeparator()) {
        ret = left + right;
    } else {
        ret = left + getOsSeparator() + right;
    }
    return ret;
}

void FileSystem::setSearchPaths(std::vector<String> searchPaths) {
    m_searchPaths = std::move(searchPaths);
}

const std::vector<String>& FileSystem::getSearchPaths() const {
    return m_searchPaths;
}

void FileSystem::addSearchPath(const String& path) {
    m_searchPaths.push_back(path);
}

}  // namespace engine
