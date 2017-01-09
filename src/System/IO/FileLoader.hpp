#pragma once

#include <Util/Precompiled.hpp>

namespace engine {

namespace io {

class FileLoader {
public:
    static bool LoadFile(const String& filename, String* dest);
};

}  // namespace io

}  // namespace engine
