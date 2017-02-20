#pragma once

#include <Util/Prerequisites.hpp>

#include <Graphics/Color32.hpp>
#include <Math/Math.hpp>
#include <System/String.hpp>

namespace engine {

class ENGINE_API Image {
public:
    Image();

    bool LoadFromFile(const String& filename);

    bool LoadFromFileInMemory(const byte* buffer, uint32 len);

    bool LoadFromMemory(const Color32* color_map, uint32 width, uint32 height);

    void Clear();

    const math::Vector2<uint32>& GetSize() const;

    byte* GetData();
    const byte* GetData() const;

private:
    math::Vector2<uint32> m_size;
    std::vector<byte> m_pixels;
};

}  // namespace engine
