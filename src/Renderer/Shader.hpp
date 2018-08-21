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

    UniformBufferObject& GetUBO();

protected:
    virtual void SetDescriptor(json&& descriptor);
    const json& GetDescriptor();

    UniformBufferObject m_ubo;

private:
    json m_descriptor;
};

}  // namespace engine
