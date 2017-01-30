#pragma once

#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstring>

#include <bitset>
#include <deque>
#include <functional>
#include <list>
#include <map>
#include <queue>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include <algorithm>
#include <chrono>
#include <numeric>
#include <random>
#include <type_traits>

#include <Util/Config.hpp>
#include <Util/NonCopyable.hpp>
#include <Util/Platform.hpp>
#include <Util/Singleton.hpp>
#include <Util/Types.hpp>

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

#include <mathfu/matrix.h>
#include <mathfu/matrix_4x4.h>
#include <mathfu/vector_2.h>
#include <mathfu/vector_3.h>
#include <mathfu/vector_4.h>

#include <utf8.h>
