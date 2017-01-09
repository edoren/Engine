#pragma once

#include <Graphics/Color32.hpp>
#include <Util/Precompiled.hpp>

namespace engine {

class Image {
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
    math::Vector2<uint32> size_;
    std::vector<byte> pixels_;
};

}  // namespace engine
