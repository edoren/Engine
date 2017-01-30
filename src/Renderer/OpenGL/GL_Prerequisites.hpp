#include <Util/Prerequisites.hpp>

#pragma once

// Include the right OpenGL libraries in each platform
#if PLATFORM_TYPE_IS(PLATFORM_TYPE_DESKTOP)
#include <GL/glew.h>
#elif PLATFORM_IS(PLATFORM_ANDROID)
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#elif PLATFORM_IS(PLATFORM_IOS)
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#endif
