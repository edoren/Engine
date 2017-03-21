#pragma once

#include <Util/Prerequisites.hpp>

#include <Math/Math.hpp>
#include <System/String.hpp>

namespace engine {

enum class ShaderType {
    Vertex,
    Fragment,
    Geometry  // TODO: Add support
};

class ENGINE_API Shader : NonCopyable {
public:
    Shader() {}
    virtual ~Shader() {}

    virtual Shader& operator=(Shader&& other) = default;

    virtual bool LoadFromMemory(const byte* source, std::size_t source_size,
                                ShaderType type) = 0;

    virtual bool Link() = 0;

    virtual void Use() = 0;
};

}  // namespace engine
