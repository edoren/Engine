#include "GL_Shader.hpp"
#include "GL_Utilities.hpp"

#include <System/LogManager.hpp>
#include <Util/Container/Vector.hpp>
#include <System/StringView.hpp>

#include <array>
#include <string>

namespace engine::plugin::opengl {

namespace {

const StringView sTag("GL_Shader");

const std::array<GLenum, 3> sGlShaderTypes = {{
    GL_VERTEX_SHADER,
    GL_FRAGMENT_SHADER,
    GL_GEOMETRY_SHADER,
}};

#ifdef OPENGL_USE_GL
const String sShaderVersion("#version 450 core");
#else
const String sShaderVersion("#version 320 es");
const String sFragmentPrecision("precision highp float;");
#endif

Vector<const char*> sRequiredExtensions = {
#ifdef OPENGL_USE_GL
    {"GL_ARB_separate_shader_objects", "GL_ARB_shading_language_420pack"}
#endif
};

}  // namespace

GL_Shader::GL_Shader() : m_program(glCreateProgram()) {
    for (unsigned int& shader : m_shaders) {
        shader = 0;
    }
    GL_CALL(glGenBuffers(1, &m_uniformBuffers.staticBuffer));
    GL_CALL(glGenBuffers(1, &m_uniformBuffers.dynamicBuffer));
}

GL_Shader::GL_Shader(GL_Shader&& other) noexcept
      : m_program(other.m_program),
        m_shaders(other.m_shaders),
        m_uniforms(std::move(other.m_uniforms)) {
    other.m_program = 0;
    for (unsigned int& shader : other.m_shaders) {
        shader = 0;
    }
}

GL_Shader::~GL_Shader() {
    if (!isLinked()) {
        cleanUpShaders();
    }
    if (m_uniformBuffers.staticBuffer) {
        GL_CALL(glDeleteBuffers(1, &m_uniformBuffers.staticBuffer));
    }
    if (m_uniformBuffers.dynamicBuffer) {
        GL_CALL(glDeleteBuffers(1, &m_uniformBuffers.dynamicBuffer));
    }
    GL_CALL(glDeleteProgram(m_program));
}

GL_Shader& GL_Shader::operator=(GL_Shader&& other) noexcept {
    new (this) GL_Shader(std::move(other));
    return *this;
}

bool GL_Shader::loadFromMemory(const byte* source, size_t sourceSize, ShaderType type) {
    if (isLinked()) {
        return false;
    }

    String sourceComplete = sShaderVersion + '\n';
#ifdef OPENGL_USE_GLES
    if (type == ShaderType::FRAGMENT) {
        source_complete += sFragmentPrecision + '\n';
    }
#endif
    for (auto& extension : sRequiredExtensions) {
        sourceComplete += String("#extension ") + extension + " : enable\n";
    }
    sourceComplete +=
        String::FromUtf8(reinterpret_cast<const char*>(source), reinterpret_cast<const char*>(source) + sourceSize);

    GLuint shader = compile(sourceComplete.getData(), sourceComplete.getSize(), type);
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

UniformBufferObject& GL_Shader::getUbo() {
    return m_ubo;
}

UniformBufferObject& GL_Shader::getUboDynamic() {
    return m_uboDynamic;
}

bool GL_Shader::isLinked() const {
    GLint success = GL_TRUE;
    GL_CALL(glGetProgramiv(m_program, GL_LINK_STATUS, &success));
    return success == GL_TRUE;
}

bool GL_Shader::link() {
    if (isLinked()) {
        return true;
    }

    GL_CALL(glLinkProgram(m_program));

    GLint success = GL_TRUE;
    GL_CALL(glGetProgramiv(m_program, GL_LINK_STATUS, &success));
    if (success == GL_FALSE) {
        GLint logSize = 0;
        GL_CALL(glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &logSize));
        std::string error;
        error.resize(logSize);
        GL_CALL(glGetProgramInfoLog(m_program, logSize, &logSize, const_cast<char*>(error.data())));
        LogError(sTag, String("Error linking shader:\n") + error);
        return false;
    }

    // After the program is succesfully linked the shaders can be cleaned
    cleanUpShaders();

    return true;
}

void GL_Shader::use() {
    if (link()) {
        GL_CALL(glUseProgram(m_program));
    }
}

void GL_Shader::uploadUniformBuffers() {
    GLuint bindingPoint;
    GLuint blockIndex;

    // TODO: Automatically detect this using the descriptor
    bindingPoint = 0;
    blockIndex = glGetUniformBlockIndex(m_program, "UniformBufferObject");
    if (blockIndex == GL_INVALID_INDEX) {
        LogError(sTag, "Error finding the UniformBufferObject with binding 0");
        return;
    }

    // Load the static UBO
    GL_CALL(glUniformBlockBinding(m_program, blockIndex, bindingPoint));
    GL_CALL(glBindBuffer(GL_UNIFORM_BUFFER, m_uniformBuffers.staticBuffer));

    GL_CALL(glBufferData(GL_UNIFORM_BUFFER, m_ubo.getDataSize(), m_ubo.getData(), GL_DYNAMIC_DRAW));
    GL_CALL(glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, m_uniformBuffers.staticBuffer));

    // TODO: Automatically detect this using the descriptor
    bindingPoint = 1;
    blockIndex = glGetUniformBlockIndex(m_program, "UniformBufferObjectDynamic");
    if (blockIndex == GL_INVALID_INDEX) {
        LogError(sTag, "Error finding the UniformBufferObjectDynamic with binding 1");
        return;
    }

    // Load the dynamic UBO
    GL_CALL(glUniformBlockBinding(m_program, blockIndex, bindingPoint));
    GL_CALL(glBindBuffer(GL_UNIFORM_BUFFER, m_uniformBuffers.dynamicBuffer));

    GL_CALL(glBufferData(GL_UNIFORM_BUFFER, m_uboDynamic.getDataSize(), m_uboDynamic.getData(), GL_DYNAMIC_DRAW));
    GL_CALL(glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, m_uniformBuffers.dynamicBuffer));
}

void GL_Shader::setUniform(const String& name, float val) {
    GLint location = getUniformLocation(name);
    if (location != -1) GL_CALL(glUniform1f(location, val));
}

void GL_Shader::setUniform(const String& name, int32 val) {
    GLint location = getUniformLocation(name);
    if (location != -1) GL_CALL(glUniform1i(location, val));
}

void GL_Shader::setUniform(const String& name, uint32 val) {
    GLint location = getUniformLocation(name);
    if (location != -1) GL_CALL(glUniform1ui(location, val));
}

void GL_Shader::setUniform(const String& name, const math::mat4& val) {
    GLint location = getUniformLocation(name);
    if (location != -1) GL_CALL(glUniformMatrix4fv(location, 1, GL_FALSE, &val[0]));
}

void GL_Shader::setUniform(const String& name, const math::mat3& val) {
    GLint location = getUniformLocation(name);
    if (location != -1) GL_CALL(glUniformMatrix3fv(location, 1, GL_FALSE, &val[0]));
}

void GL_Shader::setUniform(const String& name, const math::mat2& val) {
    GLint location = getUniformLocation(name);
    if (location != -1) GL_CALL(glUniformMatrix2fv(location, 1, GL_FALSE, &val[0]));
}

void GL_Shader::setUniform(const String& name, const math::vec4& val) {
    GLint location = getUniformLocation(name);
    if (location != -1) GL_CALL(glUniform4f(location, val[0], val[1], val[2], val[3]));
}

void GL_Shader::setUniform(const String& name, const math::vec3& val) {
    GLint location = getUniformLocation(name);
    if (location != -1) GL_CALL(glUniform3f(location, val[0], val[1], val[2]));
}

void GL_Shader::setUniform(const String& name, const math::vec2& val) {
    GLint location = getUniformLocation(name);
    if (location != -1) GL_CALL(glUniform2f(location, val[0], val[1]));
}

const Vector<const char*>& GL_Shader::GetRequiredExtensions() {
    return sRequiredExtensions;
}

void GL_Shader::setDescriptor(json&& descriptor) {
    m_descriptor = std::move(descriptor);

    Vector<UniformBufferObject::Item> attributes;
    for (auto& attribute : m_descriptor["uniform_buffer"]["attributes"]) {
        String name = attribute["name"];
        String type = attribute["type"];
        attributes.push_back({name, getUboDataTypeFromString(type)});
    }
    m_ubo.setAttributes(attributes);

    attributes.clear();
    for (auto& attribute : m_descriptor["uniform_buffer_dynamic"]["attributes"]) {
        String name = attribute["name"];
        String type = attribute["type"];
        attributes.push_back({name, getUboDataTypeFromString(type)});
    }
    m_uboDynamic.setAttributes(attributes);
}

GLuint GL_Shader::compile(const char* source, size_t sourceSize, ShaderType type) {
    return compile(reinterpret_cast<const byte*>(source), sourceSize, type);
}

GLuint GL_Shader::compile(const byte* source, size_t sourceSize, ShaderType type) {
    if (source == nullptr || sourceSize == 0) {
        return 0;
    }

    GLuint shader = glCreateShader(sGlShaderTypes[static_cast<int>(type)]);

    const char* sourceStr = reinterpret_cast<const char*>(source);
    auto length = static_cast<GLint>(sourceSize);
    GL_CALL(glShaderSource(shader, 1, &sourceStr, &length));
    GL_CALL(glCompileShader(shader));

    GLint success = 0;
    GL_CALL(glGetShaderiv(shader, GL_COMPILE_STATUS, &success));
    if (success == GL_FALSE) {
        GLint logSize = 0;
        GL_CALL(glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize));
        std::string error;
        error.resize(logSize);
        GL_CALL(glGetShaderInfoLog(shader, logSize, &logSize, const_cast<char*>(error.data())));
        LogError(sTag, String("Error compiling shader:\n") + error);
        GL_CALL(glDeleteShader(shader));
        return 0;
    }

    return shader;
}

void GL_Shader::cleanUpShaders() {
    for (unsigned int& mShader : m_shaders) {
        if (mShader) {
            GL_CALL(glDetachShader(m_program, mShader));
            GL_CALL(glDeleteShader(mShader));
            mShader = 0;
        }
    }
}

GLint GL_Shader::getUniformLocation(const String& name) {
    const auto it = m_uniforms.find(name);

    if (it != m_uniforms.end()) {
        return it->second;
    }
    GLint location = glGetUniformLocation(m_program, name.toUtf8().c_str());

    m_uniforms.insert(std::make_pair(name, location));

    if (location == -1) {
        LogError(sTag, "Parameter \"{}\" not found in shader", name);
    }

    return location;
}

}  // namespace engine::plugin::opengl
