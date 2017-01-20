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

#include <Util/Platform.hpp>
#include <Util/Config.hpp>
#include <Util/NonCopyable.hpp>
#include <Util/Singleton.hpp>
#include <Util/Types.hpp>

#include <System/String.hpp>

#include <Math/GLSLMappings.hpp>
#include <Math/Geometric.hpp>
#include <Math/MatrixTransform.hpp>
#include <Math/Trigonometric.hpp>

#define USING_STD_FILESYSTEM

#ifdef USING_STD_FILESYSTEM
#if PLATFORM == PLATFORM_WINDOWS
#include <filesystem>
namespace external {
namespace fs = std::experimental::filesystem;
}
#endif
#endif

#include <SDL.h>
#include <SDL_log.h>
