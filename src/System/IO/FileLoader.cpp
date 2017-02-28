#include <System/IO/FileLoader.hpp>
#include <System/LogManager.hpp>

#include <SDL.h>

namespace engine {

namespace io {

bool FileLoader::LoadFile(const String& filename, String* dest) {
    std::vector<byte> out;
    bool success = LoadFile(filename, &out);
    if (success) {
        char8* begin = reinterpret_cast<char8*>(out.data());
        *dest = String::FromUtf8(begin, begin + out.size());
    }
    return success;
}

bool FileLoader::LoadFile(const String& filename, std::vector<byte>* dest) {
    SDL_RWops* handle = SDL_RWFromFile(filename.GetData(), "rb");
    if (!handle) {
        LogError("FileLoader", "LoadFile fail on " + filename);
        return false;
    }
    std::size_t len =
        static_cast<std::size_t>(SDL_RWseek(handle, 0, RW_SEEK_END));
    SDL_RWseek(handle, 0, RW_SEEK_SET);
    dest->resize(len);
    std::size_t rlen =
        static_cast<std::size_t>(SDL_RWread(handle, &(*dest)[0], 1, len));
    SDL_RWclose(handle);
    return len == rlen && len > 0;
}

}  // namespace io

}  // namespace engine
