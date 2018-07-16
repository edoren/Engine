#include <System/FileSystem.hpp>
#include <System/IOStream.hpp>

#include <SDL2.h>

namespace engine {

IOStream::IOStream() : m_file(nullptr) {}

IOStream::IOStream(IOStream&& other)
      : m_file(other.m_file), m_last_error(std::move(other.m_last_error)) {
    other.m_file = nullptr;
    other.m_last_error.Clear();
}

IOStream::~IOStream() {
    if (m_file) {
        SDL_RWclose(m_file);
        m_file = nullptr;
    }
}

IOStream& IOStream::operator=(IOStream&& other) {
    m_file = other.m_file;
    m_last_error = std::move(other.m_last_error);
    other.m_file = nullptr;
    other.m_last_error.Clear();
    return *this;
}

bool IOStream::Open(const String& filename, const char* mode) {
    if (m_file) {
        Close();
    }
    m_file = SDL_RWFromFile(filename.GetData(), mode);
    if (!m_file) {
        m_last_error = SDL_GetError();
        return false;
    }
    return true;
}

bool IOStream::Open(const char* filename, const char* mode) {
    return Open(String(filename), mode);
}

void IOStream::Close() {
    SDL_RWclose(m_file);
    m_file = nullptr;
}

size_t IOStream::Read(void* buffer, size_t size, size_t count) {
    return m_file ? SDL_RWread(m_file, buffer, size, count) : 0;
}

size_t IOStream::Write(const void* buffer, size_t size, size_t count) {
    return m_file ? SDL_RWwrite(m_file, buffer, size, count) : 0;
}

int64 IOStream::Seek(size_t offset, Origin origin) {
    return m_file ? SDL_RWseek(m_file, offset, static_cast<int>(origin)) : 0;
}

int64 IOStream::Tell() const {
    return m_file ? SDL_RWtell(m_file) : 0;
}

size_t IOStream::GetSize() const {
    if (!m_file) return 0;
    int64 current_pos = SDL_RWtell(m_file);
    SDL_RWseek(m_file, 0, RW_SEEK_END);
    int64 file_size = SDL_RWtell(m_file);
    SDL_RWseek(m_file, current_pos, RW_SEEK_SET);
    return static_cast<size_t>(file_size);
}

bool IOStream::IsOpen() const {
    return m_file != nullptr;
}

const String& IOStream::GetLastError() const {
    return m_last_error;
}

}  // namespace engine
