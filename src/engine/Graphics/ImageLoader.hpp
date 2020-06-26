#pragma once

#include <Util/Prerequisites.hpp>

#include <Math/Math.hpp>
#include <System/String.hpp>
#include <Util/Container/Vector.hpp>

namespace engine {

namespace io {

class ENGINE_API ImageLoader {
public:
    static bool LoadFromFile(const String& filename, Vector<byte>& pixels, math::uvec2& size);

    static bool LoadFromFileInMemory(const byte* buffer, uint32 len, Vector<byte>& pixels, math::uvec2& size);
};

}  // namespace io

}  // namespace engine
