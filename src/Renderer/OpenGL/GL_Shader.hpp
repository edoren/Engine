#pragma once

#include <Renderer/OpenGL/GL_Utilities.hpp>

namespace engine {

class Shader : NonCopyable {
public:
    Shader();
    Shader(Shader&& other);
    ~Shader();

    Shader& operator=(Shader&& other);

    int LoadFromMemory(const String& vertex_source,
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
    GLuint Compile(GLenum type, const char* source);

    int CompileAndLink(const char* vertex_source, const char* fragment_source);

    GLint GetUniformLocation(const String& name);

private:
    GLuint program_;
    std::map<String, GLint> uniforms_;
};

}  // namespace engine
