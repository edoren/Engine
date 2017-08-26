#pragma once

#include <Util/Prerequisites.hpp>

#include <System/String.hpp>

class SDL_RWops;

namespace engine {

class ENGINE_API IOStream : NonCopyable {
public:
    enum Origin : int { SET = 0, CUR = 1, END = 2 };

    IOStream();
    IOStream(IOStream&& other);

    ~IOStream();

    IOStream& operator=(IOStream&& other);

    bool Open(const String& filename, const char* mode);
    bool Open(const char8* filename, const char* mode);

    void Close();

    size_t Read(void* buffer, size_t size, size_t count);

    size_t Write(const void* buffer, size_t size, size_t count);

    int64 Seek(size_t offset, Origin origin);

    size_t Tell() const;

    size_t GetSize() const;

    bool IsOpen() const;

    const String& GetLastError() const;

private:
    SDL_RWops* m_file;
    String m_last_error;
};

}  // namespace engine
