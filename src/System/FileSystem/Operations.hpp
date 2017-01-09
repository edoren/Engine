#pragma once

#include <Util/Precompiled.hpp>
#include <System/FileSystem/Path.hpp>

namespace engine {

namespace filesystem {

Path CurrentDirectory();

Path Absolute(const Path& path, const Path& base = CurrentDirectory());

}  // namespace filesystem

}  // namespace engine
