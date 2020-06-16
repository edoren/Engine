#pragma once

#include <Graphics/Image.hpp>

namespace engine {

class ENGINE_API Texture2D : NonCopyable {
public:
    Texture2D() = default;

    virtual ~Texture2D() = default;

    virtual bool loadFromImage(const Image& img) = 0;

    virtual void use() = 0;
};

}  // namespace engine
