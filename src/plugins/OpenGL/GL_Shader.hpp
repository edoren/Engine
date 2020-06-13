#pragma once

#include <Math/Math.hpp>
#include <Renderer/Shader.hpp>
#include <Renderer/UniformBufferObject.hpp>
#include <System/String.hpp>

#include "GL_Config.hpp"
#include "GL_Dependencies.hpp"

namespace engine {

class OPENGL_PLUGIN_API GL_Shader : public Shader {
public:
    GL_Shader();
    GL_Shader(GL_Shader&& other);
    ~GL_Shader() override;

    GL_Shader& operator=(GL_Shader&& other);

    bool LoadFromMemory(const byte* source, std::size_t source_size, ShaderType type) override;

    UniformBufferObject& GetUBO();
    UniformBufferObject& GetUBODynamic();

    bool IsLinked() const;

    bool Link();

    void Use();

    void UploadUniformBuffers();

    void SetUniform(const String& name, float val);
    void SetUniform(const String& name, int32 val);
    void SetUniform(const String& name, uint32 val);
    void SetUniform(const String& name, const math::mat4& val);
    void SetUniform(const String& name, const math::mat3& val);
    void SetUniform(const String& name, const math::mat2& val);
    void SetUniform(const String& name, const math::vec4& val);
    void SetUniform(const String& name, const math::vec3& val);
    void SetUniform(const String& name, const math::vec2& val);

    static const std::vector<const char*>& GetRequiredExtensions();

protected:
    void SetDescriptor(json&& descriptor) override;

private:
    GLuint Compile(const char8* source, std::size_t source_size, ShaderType type);
    GLuint Compile(const byte* source, std::size_t source_size, ShaderType type);

    void CleanUpShaders();

    GLint GetUniformLocation(const String& name);

private:
    json m_descriptor;

    UniformBufferObject m_ubo;
    UniformBufferObject m_ubo_dynamic;

    GLuint m_program;
    std::array<GLuint, sShaderTypeCount> m_shaders;

    struct {
        GLuint _static;
        GLuint _dynamic;
    } m_uniform_buffers;

    std::map<String, GLint> m_uniforms;
};

}  // namespace engine
