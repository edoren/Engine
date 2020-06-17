#include "GL_Utilities.hpp"
#include "GL_Dependencies.hpp"

#include <System/LogManager.hpp>

namespace engine {

namespace {

const String sTag("GL_Utilities");

}  // namespace

void LogGLError(const char* file, int line, const char* call) {
    auto err = glGetError();
    if (err == GL_NO_ERROR) {
        return;
    }
    const char* errStr = "<unknown error enum>";
    switch (err) {
        case GL_INVALID_ENUM:
            errStr = "GL_INVALID_ENUM";
            break;
        case GL_INVALID_VALUE:
            errStr = "GL_INVALID_VALUE";
            break;
        case GL_INVALID_OPERATION:
            errStr = "GL_INVALID_OPERATION";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            errStr = "GL_INVALID_FRAMEBUFFER_OPERATION";
            break;
        case GL_OUT_OF_MEMORY:
            errStr = "GL_OUT_OF_MEMORY";
            break;
    }

    char buffer[512];
    sprintf(buffer, "%s(%d): OpenGL Error: %s from %s", file, line, errStr, call);

    LogError(sTag, buffer);
    assert(0);
};

}  // namespace engine
