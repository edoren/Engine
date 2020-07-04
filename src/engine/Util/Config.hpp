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
#elif __GNUC__ >= 4
    // GCC 4 has special keywords for showing/hidding symbols,
    // the same keyword is used for both importing and exporting
    #define ENGINE_SYMBOL_EXPORTS __attribute__((__visibility__("default")))
    #define ENGINE_SYMBOL_IMPORTS __attribute__((__visibility__("default")))
#else
    // GCC < 4 has no mechanism to explicitely hide symbols, everything's
    // exported
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

/**
 * @brief Namespace that contains all the vanilla engine code
 */
namespace engine {}

/**
 * @brief Namespace that contains all the provided plugins
 */
namespace engine::plugin {}

/**
 * @brief Namespace that contains functions and classes to handle UTF encodings
 */
namespace engine::utf {}
