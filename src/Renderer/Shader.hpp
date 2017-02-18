#pragma once

#include <Util/Prerequisites.hpp>

#include <Math/Math.hpp>
#include <System/String.hpp>

namespace engine {

class ENGINE_API Shader : NonCopyable {
public:
    Shader() {}
    virtual ~Shader() {}

    virtual Shader& operator=(Shader&& other) = default;

    virtual bool LoadFromMemory(const String& vertex_source,
                                const String& fragment_source) = 0;

    virtual void Use() = 0;
};

}  // namespace engine
