#include "GL_Shader.hpp"
#include "GL_Utilities.hpp"

#include <System/LogManager.hpp>

namespace engine {

namespace {

const String sTag("GL_Shader");

const std::array<GLenum, 3> sGlShaderTypes = {
    GL_VERTEX_SHADER, GL_FRAGMENT_SHADER, GL_GEOMETRY_SHADER};

}  // namespace

GL_Shader::GL_Shader() : m_program(glCreateProgram()) {
    for (size_t i = 0; i < m_shaders.size(); i++) {
        m_shaders[i] = 0;
    }
}

GL_Shader::GL_Shader(GL_Shader&& other)
      : m_program(other.m_program),
        m_shaders(std::move(other.m_shaders)),
        m_uniforms(std::move(other.m_uniforms)) {
    other.m_program = 0;
    for (size_t i = 0; i < other.m_shaders.size(); i++) {
        other.m_shaders[i] = 0;
    }
}

GL_Shader::~GL_Shader() {
    if (!IsLinked()) CleanUpShaders();
    GL_CALL(glDeleteProgram(m_program));
}

GL_Shader& GL_Shader::operator=(GL_Shader&& other) {
    m_program = other.m_program;
    m_shaders = std::move(other.m_shaders);
    m_uniforms = std::move(other.m_uniforms);
    other.m_program = 0;
    for (size_t i = 0; i < other.m_shaders.size(); i++) {
        other.m_shaders[i] = 0;
    }
    return *this;
}

bool GL_Shader::LoadFromMemory(const byte* source, std::size_t source_size,
                               ShaderType type) {
    if (IsLinked()) {
        return false;
    }

    GLuint shader = Compile(source, source_size, type);
    if (!shader) {
        return false;
    }

    size_t pos = static_cast<GLuint>(type);

    if (m_shaders[pos]) {
        GL_CALL(glDetachShader(m_program, m_shaders[pos]));
        GL_CALL(glDeleteShader(m_shaders[pos]));
        m_shaders[pos] = 0;
    }

    GL_CALL(glAttachShader(m_program, shader));
    m_shaders[pos] = shader;

    return true;
}

bool GL_Shader::IsLinked() {
    GLint success = GL_TRUE;
    GL_CALL(glGetProgramiv(m_program, GL_LINK_STATUS, &success));
    return success == GL_TRUE;
}

bool GL_Shader::Link() {
    if (IsLinked()) {
        return true;
    }

    GL_CALL(glLinkProgram(m_program));

    GLint success = GL_TRUE;
    GL_CALL(glGetProgramiv(m_program, GL_LINK_STATUS, &success));
    if (success == GL_FALSE) {
        GLint log_size = 0;
        GL_CALL(glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &log_size));
        char* error = new char[log_size];
        GL_CALL(glGetProgramInfoLog(m_program, log_size, &log_size, error));
        LogError(sTag, String("Error linking shader:\n") + error);
        delete[] error;
        return false;
    }

    // After the program is succesfully linked the shaders can be cleaned
    CleanUpShaders();

    return true;
}

void GL_Shader::Use() {
    if (Link()) {
        GL_CALL(glUseProgram(m_program));
    }
    if (m_uniform_buffer) {
        GL_CALL(glDeleteBuffers(1, &m_uniform_buffer));
    }
}

void GL_Shader::SetUniformBufferObject(const UniformBufferObject& ubo) {
    GLuint binding_point = 0;
    GLuint block_index =
        glGetUniformBlockIndex(m_program, "UniformBufferObject");
    if (block_index == GL_INVALID_INDEX) {
        LogError(sTag, "Error finding the UniformBufferObject");
        return;
    }

    GL_CALL(glUniformBlockBinding(m_program, block_index, binding_point));
    GL_CALL(glGenBuffers(1, &m_uniform_buffer));
    GL_CALL(glBindBuffer(GL_UNIFORM_BUFFER, m_uniform_buffer));

    GL_CALL(glBufferData(GL_UNIFORM_BUFFER, sizeof(UniformBufferObject), &ubo,
                         GL_DYNAMIC_DRAW));
    GL_CALL(
        glBindBufferBase(GL_UNIFORM_BUFFER, binding_point, m_uniform_buffer));
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

GLuint GL_Shader::Compile(const byte* source, size_t source_size,
                          ShaderType type) {
    if (source == nullptr || source_size == 0) return 0;

    GLuint shader = glCreateShader(sGlShaderTypes[static_cast<int>(type)]);

    const char* source_str = reinterpret_cast<const char*>(source);
    GLint length = static_cast<GLint>(source_size);
    GL_CALL(glShaderSource(shader, 1, &source_str, &length));
    GL_CALL(glCompileShader(shader));

    GLint success = 0;
    GL_CALL(glGetShaderiv(shader, GL_COMPILE_STATUS, &success));
    if (success == GL_FALSE) {
        GLint log_size = 0;
        GL_CALL(glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_size));
        char* error = new char[log_size];
        GL_CALL(glGetShaderInfoLog(shader, log_size, &log_size, error));
        LogError(sTag, String("Error compiling shader:\n") + error);
        delete[] error;
        GL_CALL(glDeleteShader(shader));
        return 0;
    }

    return shader;
}

void GL_Shader::CleanUpShaders() {
    for (size_t i = 0; i < m_shaders.size(); i++) {
        if (m_shaders[i]) {
            GL_CALL(glDetachShader(m_program, m_shaders[i]));
            GL_CALL(glDeleteShader(m_shaders[i]));
            m_shaders[i] = 0;
        }
    }
}

GLint GL_Shader::GetUniformLocation(const String& name) {
    const auto it = m_uniforms.find(name);

    if (it != m_uniforms.end()) {
        return it->second;
    } else {
        GLint location = glGetUniformLocation(m_program, name.ToUtf8().c_str());
        m_uniforms.insert(std::make_pair(name, location));

        if (location == -1) {
            LogError(sTag, "Parameter \"" + name + "\" not found in shader");
        }

        return location;
    }

    return -1;
}

}  // namespace engine
