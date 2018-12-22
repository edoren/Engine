#pragma once

#include <Util/Prerequisites.hpp>

#include <Renderer/UniformBufferObject.hpp>
#include <System/JSON.hpp>

namespace engine {

enum class ShaderType {
    VERTEX,
    FRAGMENT,
    GEOMETRY  // Not used currently
};

static const size_t sShaderTypeCount = 3;

class ENGINE_API Shader : NonCopyable {
    friend class ShaderManager;

public:
    Shader();
    virtual ~Shader();

    virtual bool LoadFromMemory(const byte* source, std::size_t source_size,
                                ShaderType type) = 0;

protected:
    virtual void SetDescriptor(json&& descriptor) = 0;

    UniformBufferObject::DataType GetUBODataTypeFromString(const String& str);
};

}  // namespace engine
