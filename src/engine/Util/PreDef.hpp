#pragma once

#include <cassert>

// clang-format off

#define ENGINE_UNUSED(var) (void)var

#ifndef ENGINE_DEBUG
    #define ENGINE_NOEXCEPTION
#endif

// Use (void) to silent unused warnings.
#define ENGINE_ASSERT(exp, msg) assert(((void)msg, exp))

#if (defined(__cpp_exceptions) || \
     defined(__EXCEPTIONS) || \
     defined(_CPPUNWIND)) &&  \
    !defined(JSON_NOEXCEPTION)
    #define ENGINE_THROW(exception) throw exception
    #define ENGINE_TRY try
    #define ENGINE_CATCH(exception) catch (exception)
#else
    #define ENGINE_THROW(exception) std::abort()
    #define ENGINE_TRY if (true)
    #define ENGINE_CATCH(exception) if (false)
#endif

// clang-format on
