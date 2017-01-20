#pragma once

#include <Util/Platform.hpp>

// Define DLL import/export macros (only Windows, and only dynamic configuration)
#if PLATFORM_IS(PLATFORM_WINDOWS) && !defined(ENGINE_STATIC)
#define ENGINE_SYMBOL_EXPORTS __declspec(dllexport)
#define ENGINE_SYMBOL_IMPORTS __declspec(dllimport)
// Disable annoying MSVC++ warning
#ifdef _MSC_VER
#pragma warning(disable: 4251)
#endif  // _MSC_VER
#else
#define ENGINE_SYMBOL_EXPORTS
#define ENGINE_SYMBOL_IMPORTS
#endif
