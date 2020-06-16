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

    virtual bool loadFromMemory(const byte* source, std::size_t sourceSize, ShaderType type) = 0;

protected:
    virtual void setDescriptor(json&& descriptor) = 0;

    UniformBufferObject::DataType getUboDataTypeFromString(const String& str);
};

}  // namespace engine
