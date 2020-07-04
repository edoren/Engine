#pragma once

#include <Math/Math.hpp>
#include <Renderer/Shader.hpp>
#include <Renderer/UniformBufferObject.hpp>
#include <System/String.hpp>
#include <Util/Container/Vector.hpp>

#include "GL_Config.hpp"
#include "GL_Dependencies.hpp"

#include <array>
#include <map>

namespace engine::plugin::opengl {

class OPENGL_PLUGIN_API GL_Shader : public Shader {
public:
    GL_Shader();
    GL_Shader(GL_Shader&& other) noexcept;
    ~GL_Shader() override;

    GL_Shader& operator=(GL_Shader&& other) noexcept;

    bool loadFromMemory(const byte* source, std::size_t sourceSize, ShaderType type) override;

    UniformBufferObject& getUbo();
    UniformBufferObject& getUboDynamic();

    bool isLinked() const;

    bool link();

    void use();

    void uploadUniformBuffers();

    void setUniform(const String& name, float val);
    void setUniform(const String& name, int32 val);
    void setUniform(const String& name, uint32 val);
    void setUniform(const String& name, const math::mat4& val);
    void setUniform(const String& name, const math::mat3& val);
    void setUniform(const String& name, const math::mat2& val);
    void setUniform(const String& name, const math::vec4& val);
    void setUniform(const String& name, const math::vec3& val);
    void setUniform(const String& name, const math::vec2& val);

    static const Vector<const char*>& GetRequiredExtensions();

protected:
    void setDescriptor(json&& descriptor) override;

private:
    GLuint compile(const char8* source, std::size_t sourceSize, ShaderType type);
    GLuint compile(const byte* source, std::size_t sourceSize, ShaderType type);

    void cleanUpShaders();

    GLint getUniformLocation(const String& name);

    json m_descriptor;

    UniformBufferObject m_ubo;
    UniformBufferObject m_uboDynamic;

    GLuint m_program;
    std::array<GLuint, sShaderTypeCount> m_shaders;

    struct {
        GLuint staticBuffer;
        GLuint dynamicBuffer;
    } m_uniformBuffers;

    std::map<String, GLint> m_uniforms;
};

}  // namespace engine::plugin::opengl
