#pragma once

#include <Util/Platform.hpp>

// Include the right OpenGL libraries in each platform
#if PLATFORM_TYPE_IS(PLATFORM_TYPE_DESKTOP)
    #include <GL/glew.h>
#elif PLATFORM_IS(PLATFORM_ANDROID)
    #include <GLES3/gl32.h>
    #include <GLES3/gl3ext.h>
#elif PLATFORM_IS(PLATFORM_IOS)
    #include <OpenGLES/ES2/gl.h>
    #include <OpenGLES/ES2/glext.h>
#endif

#include <SDL2.h>

#include "GL_Utilities.hpp"
