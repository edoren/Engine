#pragma once

#include <Util/Prerequisites.hpp>

#include <System/String.hpp>

namespace engine {

namespace io {

class ENGINE_API FileLoader {
public:
    static bool LoadFile(const String& filename, String* dest);
    static bool LoadFile(const String& filename, std::vector<byte>* dest);
};

}  // namespace io

}  // namespace engine
