
#include <System/IO/FileLoader.hpp>
#include <Util/Precompiled.hpp>

namespace engine {

namespace io {

bool FileLoader::LoadFile(const String& filename, String* dest) {
    SDL_RWops* handle = SDL_RWFromFile(filename.GetData(), "rb");
    if (!handle) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "LoadFile fail on %s", filename);
        return false;
    }
    std::size_t len =
        static_cast<std::size_t>(SDL_RWseek(handle, 0, RW_SEEK_END));
    SDL_RWseek(handle, 0, RW_SEEK_SET);
    char* temp = new char[len];
    std::size_t rlen =
        static_cast<std::size_t>(SDL_RWread(handle, temp, 1, len));
    *dest = String::FromUtf8(temp, temp + rlen);
    delete[] temp;
    SDL_RWclose(handle);
    return len == rlen && len > 0;
}

}  // namespace io

}  // namespace engine
