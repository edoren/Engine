#pragma once

#include <Util/Prerequisites.hpp>

namespace engine {

enum class ShaderType {
    eVertex,
    eFragment,
    eGeometry  // Not used currently
};

static const size_t sShaderTypeCount = 2;

class ENGINE_API Shader : NonCopyable {
public:
    Shader() {}

    virtual ~Shader() {}

    virtual bool LoadFromMemory(const byte* source, std::size_t source_size,
                                ShaderType type) = 0;
};

}  // namespace engine
