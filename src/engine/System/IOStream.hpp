#pragma once

#include <Util/Prerequisites.hpp>

#include <System/String.hpp>

struct SDL_RWops;

namespace engine {

class ENGINE_API IOStream : NonCopyable {
public:
    enum class Origin : int {
        SET = 0,
        CUR = 1,
        END = 2,
    };

    IOStream();
    IOStream(IOStream&& other);

    ~IOStream();

    IOStream& operator=(IOStream&& other);

    bool open(const String& filename, const char* mode);
    bool open(const char8* filename, const char* mode);

    void close();

    size_t read(void* buffer, size_t size, size_t count);

    size_t write(const void* buffer, size_t size, size_t count);

    int64 seek(size_t offset, Origin origin);

    int64 tell() const;

    size_t getSize() const;

    bool isOpen() const;

    const String& getLastError() const;

private:
    SDL_RWops* m_file;
    String m_last_error;
};

}  // namespace engine
