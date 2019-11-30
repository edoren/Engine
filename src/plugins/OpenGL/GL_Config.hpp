#pragma once

#include <Util/Config.hpp>

#ifdef OPENGL_PLUGIN_EXPORTS
    #define OPENGL_PLUGIN_API ENGINE_SYMBOL_EXPORTS
#else
    #define OPENGL_PLUGIN_API ENGINE_SYMBOL_IMPORTS
#endif

#if PLATFORM_IS(PLATFORM_ANDROID | PLATFORM_IOS)
    #define OPENGL_USE_GLES
#else
    #define OPENGL_USE_GL
#endif
