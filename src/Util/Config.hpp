#pragma once

#include <Util/Platform.hpp>

// Define DLL import/export macros (only Windows, and only dynamic
// configuration)
#if PLATFORM_IS(PLATFORM_WINDOWS) && !defined(ENGINE_STATIC)
#define ENGINE_SYMBOL_EXPORTS __declspec(dllexport)
#define ENGINE_SYMBOL_IMPORTS __declspec(dllimport)
// Disable annoying MSVC++ warning
#ifdef _MSC_VER
#pragma warning(disable : 4251)
#pragma warning(disable : 4661)
#endif  // _MSC_VER
#else
#define ENGINE_SYMBOL_EXPORTS
#define ENGINE_SYMBOL_IMPORTS
#endif

#ifdef ENGINE_EXPORTS
#define ENGINE_API ENGINE_SYMBOL_EXPORTS
#else
#define ENGINE_API ENGINE_SYMBOL_IMPORTS
#endif

#if defined(DEBUG) || !defined(NDEBUG) || defined(_DEBUG)
#define ENGINE_DEBUG
#endif

// Disable warning for not using CRT secure functions
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable : 4996)
#endif
