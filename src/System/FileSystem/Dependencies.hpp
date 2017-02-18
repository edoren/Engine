#pragma once

#define USING_STD_FILESYSTEM
#ifdef USING_STD_FILESYSTEM
#include <Util/STLHeaders.hpp>
namespace external {
namespace fs = std::experimental::filesystem;
}
#endif
