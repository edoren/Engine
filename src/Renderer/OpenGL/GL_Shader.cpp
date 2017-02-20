#include "GL_Shader.hpp"
#include "GL_Utilities.hpp"

#include <System/LogManager.hpp>

namespace engine {

GL_Shader::GL_Shader() : m_program(0) {}

GL_Shader::GL_Shader(GL_Shader&& other)
      : m_program(other.m_program), m_uniforms(std::move(other.m_uniforms)) {
    other.m_program = 0;
}

GL_Shader::~GL_Shader() {
    GL_CALL(glDeleteProgram(m_program));
}

GL_Shader& GL_Shader::operator=(GL_Shader&& other) {
    m_program = other.m_program;
    m_uniforms = std::move(other.m_uniforms);
    other.m_program = 0;
    return *this;
}

bool GL_Shader::LoadFromMemory(const String& vertex_source,
                               const String& fragment_source) {
    return CompileAndLink(vertex_source.GetData(), fragment_source.GetData());
}

void GL_Shader::Use() {
    GL_CALL(glUseProgram(m_program));
}

void GL_Shader::SetUniform(const String& name, float val) {
    GLint location = GetUniformLocation(name);
    if (location != -1) GL_CALL(glUniform1f(location, val));
}

void GL_Shader::SetUniform(const String& name, int32 val) {
    GLint location = GetUniformLocation(name);
    if (location != -1) GL_CALL(glUniform1i(location, val));
}

void GL_Shader::SetUniform(const String& name, uint32 val) {
    GLint location = GetUniformLocation(name);
    if (location != -1) GL_CALL(glUniform1ui(location, val));
}

void GL_Shader::SetUniform(const String& name, const math::mat4& val) {
    GLint location = GetUniformLocation(name);
    if (location != -1)
        GL_CALL(glUniformMatrix4fv(location, 1, GL_FALSE, &val[0]));
}

void GL_Shader::SetUniform(const String& name, const math::mat3& val) {
    GLint location = GetUniformLocation(name);
    if (location != -1)
        GL_CALL(glUniformMatrix3fv(location, 1, GL_FALSE, &val[0]));
}

void GL_Shader::SetUniform(const String& name, const math::mat2& val) {
    GLint location = GetUniformLocation(name);
    if (location != -1)
        GL_CALL(glUniformMatrix2fv(location, 1, GL_FALSE, &val[0]));
}

void GL_Shader::SetUniform(const String& name, const math::vec4& val) {
    GLint location = GetUniformLocation(name);
    if (location != -1)
        GL_CALL(glUniform4f(location, val[0], val[1], val[2], val[3]));
}

void GL_Shader::SetUniform(const String& name, const math::vec3& val) {
    GLint location = GetUniformLocation(name);
    if (location != -1) GL_CALL(glUniform3f(location, val[0], val[1], val[2]));
}

void GL_Shader::SetUniform(const String& name, const math::vec2& val) {
    GLint location = GetUniformLocation(name);
    if (location != -1) GL_CALL(glUniform2f(location, val[0], val[1]));
}

GLuint GL_Shader::Compile(GLenum shader_type, const char* source) {
    if (source == nullptr) return 0;
    GLuint shader = glCreateShader(shader_type);
    GL_CALL(glShaderSource(shader, 1, &source, nullptr));
    GL_CALL(glCompileShader(shader));
    GLint success = 0;
    GL_CALL(glGetShaderiv(shader, GL_COMPILE_STATUS, &success));
    if (success == GL_FALSE) {
        GLint log_size = 0;
        GL_CALL(glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_size));
        char* error = new char[log_size];
        GL_CALL(glGetShaderInfoLog(shader, log_size, &log_size, error));
        LogError("Shader", error);
        delete[] error;
        GL_CALL(glDeleteShader(shader));
        return 0;
    }
    return shader;
}

bool GL_Shader::CompileAndLink(const char* vertex_source,
                               const char* fragment_source) {
    if (m_program) GL_CALL(glDeleteProgram(m_program));

    GLuint vertex_shader = Compile(GL_VERTEX_SHADER, vertex_source);
    if (!vertex_shader) {
        GL_CALL(glDeleteShader(vertex_shader));
        return false;
    }
    GLuint fragment_shader = Compile(GL_FRAGMENT_SHADER, fragment_source);
    if (!fragment_shader) {
        GL_CALL(glDeleteShader(fragment_shader));
        GL_CALL(glDeleteShader(vertex_shader));
        return false;
    }

    m_program = glCreateProgram();
    GL_CALL(glAttachShader(m_program, vertex_shader));
    GL_CALL(glAttachShader(m_program, fragment_shader));
    GL_CALL(glLinkProgram(m_program));

    GLint success = 0;
    glGetProgramiv(m_program, GL_LINK_STATUS, &success);
    if (success == GL_FALSE) {
        GLint log_size = 0;
        GL_CALL(glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &log_size));
        char* error = new char[log_size];
        GL_CALL(glGetProgramInfoLog(m_program, log_size, &log_size, error));
        LogError("Shader", error);
        delete[] error;
        GL_CALL(glDeleteProgram(m_program));
        GL_CALL(glDeleteShader(vertex_shader));
        GL_CALL(glDeleteShader(fragment_shader));
        return false;
    }

    GL_CALL(glDetachShader(m_program, vertex_shader));
    GL_CALL(glDetachShader(m_program, fragment_shader));
    GL_CALL(glDeleteShader(vertex_shader));
    GL_CALL(glDeleteShader(fragment_shader));

    return true;
}

GLint GL_Shader::GetUniformLocation(const String& name) {
    const auto it = m_uniforms.find(name);

    if (it != m_uniforms.end()) {
        return it->second;
    } else {
        GLint location = glGetUniformLocation(m_program, name.ToUtf8().c_str());
        m_uniforms.insert(std::make_pair(name, location));

        if (location == -1) {
            LogError("Shader",
                     "Parameter \"" + name + "\" not found in shader");
        }

        return location;
    }

    return -1;
}

}  // namespace engine
