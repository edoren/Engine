#pragma once

#include <System/String.hpp>

namespace engine {

class ENGINE_API Plugin {
public:
    virtual ~Plugin() {}

    virtual const String& getName() const = 0;

    virtual void install() = 0;

    virtual void initialize() = 0;

    virtual void shutdown() = 0;

    virtual void uninstall() = 0;
};

}  // namespace engine
