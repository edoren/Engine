#pragma once

#include <Util/Prerequisites.hpp>

#include <System/String.hpp>

namespace engine {

class ENGINE_API SharedLibrary : NonCopyable {
public:
    explicit SharedLibrary(String name);
    SharedLibrary(SharedLibrary&& other);
    ~SharedLibrary();

    bool load();

    void unload();

    String getErrorString();

    const String& getName() const;

    void* getSymbol(const char* name);
    void* getSymbol(const String& name);

private:
    String m_name;
    void* m_handle;
};

}  // namespace engine
