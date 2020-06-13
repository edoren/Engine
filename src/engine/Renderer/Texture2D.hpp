#pragma once

#include <Graphics/Image.hpp>

namespace engine {

class ENGINE_API Texture2D : NonCopyable {
public:
    Texture2D() {}

    virtual ~Texture2D() {}

    virtual bool LoadFromImage(const Image& img) = 0;

    virtual void Use() = 0;
};

}  // namespace engine
