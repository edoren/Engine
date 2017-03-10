#pragma once

#include <Util/Prerequisites.hpp>

#include <System/String.hpp>

namespace engine {

class ENGINE_API SharedLibrary : NonCopyable {
public:
    explicit SharedLibrary(const String& name);
    SharedLibrary(SharedLibrary&& other);
    ~SharedLibrary();

    bool Load();

    void Unload();

    String GetErrorString();

    const String& GetName() const;

    void* GetSymbol(const char* name);
    void* GetSymbol(const String& name);

private:
    String m_name;
    void* m_handle;
};

}  // namespace engine
