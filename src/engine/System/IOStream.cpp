#include <System/FileSystem.hpp>
#include <System/IOStream.hpp>

#include <SDL2.h>

namespace engine {

IOStream::IOStream() : m_file(nullptr) {}

IOStream::IOStream(IOStream&& other) : m_file(other.m_file), m_lastError(std::move(other.m_lastError)) {
    other.m_file = nullptr;
    other.m_lastError.clear();
}

IOStream::~IOStream() {
    if (m_file) {
        SDL_RWclose(m_file);
        m_file = nullptr;
    }
}

IOStream& IOStream::operator=(IOStream&& other) {
    new (this) IOStream(std::move(other));
    return *this;
}

bool IOStream::open(const String& filename, const char* mode) {
    if (m_file) {
        close();
    }
    m_file = SDL_RWFromFile(filename.getData(), mode);
    if (!m_file) {
        m_lastError = SDL_GetError();
        return false;
    }
    return true;
}

bool IOStream::open(const char* filename, const char* mode) {
    return open(String(filename), mode);
}

void IOStream::close() {
    SDL_RWclose(m_file);
    m_file = nullptr;
}

size_t IOStream::read(void* buffer, size_t size, size_t count) {
    return m_file ? SDL_RWread(m_file, buffer, size, count) : 0;
}

size_t IOStream::write(const void* buffer, size_t size, size_t count) {
    return m_file ? SDL_RWwrite(m_file, buffer, size, count) : 0;
}

int64 IOStream::seek(size_t offset, Origin origin) {
    return m_file ? SDL_RWseek(m_file, offset, static_cast<int>(origin)) : 0;
}

int64 IOStream::tell() const {
    return m_file ? SDL_RWtell(m_file) : 0;
}

size_t IOStream::getSize() const {
    if (!m_file) {
        return 0;
    }
    int64 current_pos = SDL_RWtell(m_file);
    SDL_RWseek(m_file, 0, RW_SEEK_END);
    int64 file_size = SDL_RWtell(m_file);
    SDL_RWseek(m_file, current_pos, RW_SEEK_SET);
    return static_cast<size_t>(file_size);
}

bool IOStream::isOpen() const {
    return m_file != nullptr;
}

const String& IOStream::getLastError() const {
    return m_lastError;
}

}  // namespace engine
