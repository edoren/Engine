#pragma once

#include <Util/Precompiled.hpp>

namespace engine {

namespace io {

class ImageLoader {
public:
    static bool LoadFromFile(const String& filename,
                             std::vector<byte>& pixels, math::uvec2& size);

    static bool LoadFromFileInMemory(const byte* buffer, uint32 len,
                                     std::vector<byte>& pixels,
                                     math::uvec2& size);
};

}  // namespace io

}  // namespace engine
