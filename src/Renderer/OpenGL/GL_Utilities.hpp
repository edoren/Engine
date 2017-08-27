#pragma once

namespace engine {

// The error checking function used by the GL_CALL macro above,
// uses glGetError() to check for errors.
void LogGLError(const char* file, int line, const char* call);

// Define a GL_CALL macro to wrap each (void-returning) OpenGL call.
// This logs GL error when LOG_GL_ERRORS below is defined.
#ifdef ENGINE_DEBUG
#define LOG_GL_ERRORS
#endif
#ifdef LOG_GL_ERRORS
#define GL_CALL(call)                          \
    {                                          \
        call;                                  \
        LogGLError(__FILE__, __LINE__, #call); \
    }
#else
#define GL_CALL(call) call
#endif

}  // namespace engine
