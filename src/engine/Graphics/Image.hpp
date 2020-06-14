#pragma once

#include <Util/Prerequisites.hpp>

#include <Graphics/Color32.hpp>
#include <Math/Math.hpp>
#include <System/String.hpp>

namespace engine {

class ENGINE_API Image {
public:
    Image();

    bool loadFromFile(const String& filename);

    bool loadFromFileInMemory(const byte* buffer, uint32 len);

    bool loadFromMemory(const Color32* color_map, uint32 width, uint32 height);

    void clear();

    const math::Vector2<uint32>& getSize() const;

    byte* getData();
    const byte* getData() const;

    size_t getDataSize() const;

private:
    math::Vector2<uint32> m_size;
    std::vector<byte> m_pixels;
};

}  // namespace engine
