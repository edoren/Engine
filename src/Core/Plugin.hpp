#pragma once

#include <System/String.hpp>

namespace engine {

class ENGINE_API Plugin {
public:
    virtual ~Plugin() {}

    virtual const String& GetName() const = 0;

    virtual void Install() = 0;

    virtual void Initialize() = 0;

    virtual void Shutdown() = 0;

    virtual void Uninstall() = 0;
};

}  // namespace engine
