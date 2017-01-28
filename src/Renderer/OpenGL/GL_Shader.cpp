#include <Renderer/OpenGL/GL_Utilities.hpp>
#include <Renderer/OpenGL/GL_Shader.hpp>

namespace engine {

Shader::Shader() : program_(0) {}

Shader::Shader(Shader&& other)
      : program_(other.program_), uniforms_(std::move(other.uniforms_)) {
    other.program_ = 0;
}

Shader::~Shader() {
    GL_CALL(glDeleteProgram(program_));
}

Shader& Shader::operator=(Shader&& other) {
    program_ = other.program_;
    uniforms_ = std::move(other.uniforms_);
    other.program_ = 0;
    return *this;
}

int Shader::LoadFromMemory(const String& vertex_source,
                           const String& fragment_source) {
    return CompileAndLink(vertex_source.GetData(), fragment_source.GetData());
}

void Shader::Use() {
    GL_CALL(glUseProgram(program_));
}

void Shader::SetUniform(const String& name, float val) {
    GLint location = GetUniformLocation(name);
    if (location != -1) GL_CALL(glUniform1f(location, val));
}

void Shader::SetUniform(const String& name, int32 val) {
    GLint location = GetUniformLocation(name);
    if (location != -1) GL_CALL(glUniform1i(location, val));
}

void Shader::SetUniform(const String& name, uint32 val) {
    GLint location = GetUniformLocation(name);
    if (location != -1) GL_CALL(glUniform1ui(location, val));
}

void Shader::SetUniform(const String& name, const math::mat4& val) {
    GLint location = GetUniformLocation(name);
    if (location != -1)
        GL_CALL(glUniformMatrix4fv(location, 1, GL_FALSE, &val[0]));
}

void Shader::SetUniform(const String& name, const math::mat3& val) {
    GLint location = GetUniformLocation(name);
    if (location != -1)
        GL_CALL(glUniformMatrix3fv(location, 1, GL_FALSE, &val[0]));
}

void Shader::SetUniform(const String& name, const math::mat2& val) {
    GLint location = GetUniformLocation(name);
    if (location != -1)
        GL_CALL(glUniformMatrix2fv(location, 1, GL_FALSE, &val[0]));
}

void Shader::SetUniform(const String& name, const math::vec4& val) {
    GLint location = GetUniformLocation(name);
    if (location != -1)
        GL_CALL(glUniform4f(location, val[0], val[1], val[2], val[3]));
}

void Shader::SetUniform(const String& name, const math::vec3& val) {
    GLint location = GetUniformLocation(name);
    if (location != -1) GL_CALL(glUniform3f(location, val[0], val[1], val[2]));
}

void Shader::SetUniform(const String& name, const math::vec2& val) {
    GLint location = GetUniformLocation(name);
    if (location != -1) GL_CALL(glUniform2f(location, val[0], val[1]));
}

GLuint Shader::Compile(GLenum shader_type, const char* source) {
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
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s", error);
        delete[] error;
        GL_CALL(glDeleteShader(shader));
        return 0;
    }
    return shader;
}

int Shader::CompileAndLink(const char* vertex_source,
                           const char* fragment_source) {
    if (program_) GL_CALL(glDeleteProgram(program_));

    GLuint vertex_shader = Compile(GL_VERTEX_SHADER, vertex_source);
    if (!vertex_shader) {
        GL_CALL(glDeleteShader(vertex_shader));
        return 0;
    }
    GLuint fragment_shader = Compile(GL_FRAGMENT_SHADER, fragment_source);
    if (!fragment_shader) {
        GL_CALL(glDeleteShader(fragment_shader));
        GL_CALL(glDeleteShader(vertex_shader));
        return 0;
    }

    program_ = glCreateProgram();
    GL_CALL(glAttachShader(program_, vertex_shader));
    GL_CALL(glAttachShader(program_, fragment_shader));
    GL_CALL(glLinkProgram(program_));

    GLint success = 0;
    glGetProgramiv(program_, GL_LINK_STATUS, &success);
    if (success == GL_FALSE) {
        GLint log_size = 0;
        GL_CALL(glGetProgramiv(program_, GL_INFO_LOG_LENGTH, &log_size));
        char* error = new char[log_size];
        GL_CALL(glGetProgramInfoLog(program_, log_size, &log_size, error));
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s", error);
        delete[] error;
        GL_CALL(glDeleteProgram(program_));
        GL_CALL(glDeleteShader(vertex_shader));
        GL_CALL(glDeleteShader(fragment_shader));
        return 0;
    }

    GL_CALL(glDetachShader(program_, vertex_shader));
    GL_CALL(glDetachShader(program_, fragment_shader));
    GL_CALL(glDeleteShader(vertex_shader));
    GL_CALL(glDeleteShader(fragment_shader));

    return 1;
}

GLint Shader::GetUniformLocation(const String& name) {
    const auto it = uniforms_.find(name);

    if (it != uniforms_.end()) {
        return it->second;
    } else {
        GLint location = glGetUniformLocation(program_, name.ToUtf8().c_str());
        uniforms_.insert(std::make_pair(name, location));

        if (location == -1) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR,
                         "Parameter \"%s\" not found in shader", name.ToUtf8().c_str());
        }

        return location;
    }

    return -1;
}

}  // namespace engine
