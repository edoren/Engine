#include "GL_Utilities.hpp"
#include "GL_Dependencies.hpp"

#include <System/LogManager.hpp>

namespace engine {

namespace {

const String sTag("GL_Utilities");

}  // namespace

void LogGLError(const char* file, int line, const char* call) {
    auto err = glGetError();
    if (err == GL_NO_ERROR) return;
    const char* err_str = "<unknown error enum>";
    switch (err) {
        case GL_INVALID_ENUM:
            err_str = "GL_INVALID_ENUM";
            break;
        case GL_INVALID_VALUE:
            err_str = "GL_INVALID_VALUE";
            break;
        case GL_INVALID_OPERATION:
            err_str = "GL_INVALID_OPERATION";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            err_str = "GL_INVALID_FRAMEBUFFER_OPERATION";
            break;
        case GL_OUT_OF_MEMORY:
            err_str = "GL_OUT_OF_MEMORY";
            break;
    }

    char buffer[512];
    sprintf(buffer, "%s(%d): OpenGL Error: %s from %s", file, line, err_str,
            call);

    LogError(sTag, buffer);
    assert(0);
};

}  // namespace engine
