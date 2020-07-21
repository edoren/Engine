#pragma once

#include <Util/Prerequisites.hpp>

#include <System/String.hpp>
#include <Util/NonCopyable.hpp>

namespace engine {

class ENGINE_API SharedLibrary : NonCopyable {
public:
    explicit SharedLibrary(const StringView& name);
    SharedLibrary(SharedLibrary&& other) noexcept;
    ~SharedLibrary();

    bool load();

    void unload();

    String getErrorString();

    const String& getName() const;

    void* getSymbol(const StringView& name);

private:
    String m_name;
    void* m_handle;
};

}  // namespace engine
