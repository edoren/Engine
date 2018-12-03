#pragma once

#include <Util/Platform.hpp>
#include <Util/STLHeaders.hpp>
#include <Util/Types.hpp>

#include <vulkan/vulkan.h>

#include <SDL2.h>
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_vulkan.h>

// clang-format off

// On Windows undefine this anoying macros defined by windows.h
#if PLATFORM_IS(PLATFORM_WINDOWS)
    #undef ERROR
    #undef TRANSPARENT
#endif

// clang-format on
