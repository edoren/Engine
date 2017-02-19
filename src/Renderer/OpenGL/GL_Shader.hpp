#pragma once

#include <Math/Math.hpp>
#include <Renderer/Shader.hpp>
#include <System/String.hpp>

#include "GL_Config.hpp"
#include "GL_Dependencies.hpp"

namespace engine {

class OPENGL_PLUGIN_API GL_Shader : public Shader {
public:
    GL_Shader();
    GL_Shader(GL_Shader&& other);
    ~GL_Shader();

    GL_Shader& operator=(GL_Shader&& other);

    bool LoadFromMemory(const String& vertex_source,
                        const String& fragment_source);

    void Use();

    void SetUniform(const String& name, float val);
    void SetUniform(const String& name, int32 val);
    void SetUniform(const String& name, uint32 val);
    void SetUniform(const String& name, const math::mat4& val);
    void SetUniform(const String& name, const math::mat3& val);
    void SetUniform(const String& name, const math::mat2& val);
    void SetUniform(const String& name, const math::vec4& val);
    void SetUniform(const String& name, const math::vec3& val);
    void SetUniform(const String& name, const math::vec2& val);

private:
    uint32 Compile(GLenum type, const char* source);

    bool CompileAndLink(const char* vertex_source, const char* fragment_source);

    int32 GetUniformLocation(const String& name);

private:
    uint32 program_;
    std::map<String, int32> uniforms_;
};

}  // namespace engine
