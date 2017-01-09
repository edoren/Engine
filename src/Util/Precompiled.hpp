#pragma once

#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstring>

#include <algorithm>
#include <functional>
#include <unordered_map>
#include <map>
#include <vector>

#include <Util/NonCopyable.hpp>
#include <Util/Platform.hpp>
#include <Util/Types.hpp>

// Define DLL import/export macros (only Windows, and only dynamic configuration)
#if PLATFORM_IS(PLATFORM_WINDOWS) && !defined(ENGINE_STATIC)
#ifdef ENGINE_IMPORTS
#define ENGINE_API __declspec(dllimport)
#else
#define ENGINE_API __declspec(dllexport)
#endif  // ENGINE_IMPORTS
// Disable annoying MSVC++ warning
#ifdef _MSC_VER
#pragma warning(disable: 4251)
#endif  // _MSC_VER
#else
#define ENGINE_API
#endif

#include <System/String.hpp>

#include <Math/GLSLMappings.hpp>
#include <Math/Geometric.hpp>
#include <Math/MatrixTransform.hpp>
#include <Math/Trigonometric.hpp>

#include <SDL.h>
#include <SDL_log.h>

#define USING_STD_FILESYSTEM

#ifdef USING_STD_FILESYSTEM
#if PLATFORM == PLATFORM_WINDOWS
#include <filesystem>
namespace external {
namespace fs = std::experimental::filesystem;
}
#endif
#endif

#ifndef USING_STD_FILESYSTEM
#if PLATFORM == PLATFORM_WINDOWS
#include <direct.h>  // for _chdir
#define getcwd _getcwd
#define chdir _chdir
#else                // PLATFORM != PLATFORM_WINDOWS
#include <unistd.h>  // for chdir
#endif
#endif

// Include the right OpenGL libraries in each platform
#if PLATFORM_TYPE == PLATFORM_TYPE_DESKTOP
#include <GL/glew.h>
#elif PLATFORM == PLATFORM_ANDROID
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#elif PLATFORM == PLATFORM_IOS
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#endif
