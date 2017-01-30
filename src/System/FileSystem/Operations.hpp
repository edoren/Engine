#pragma once

#include <Util/Prerequisites.hpp>

#include <System/FileSystem/Path.hpp>

namespace engine {

namespace filesystem {

ENGINE_API Path CurrentDirectory();

ENGINE_API Path Absolute(const Path& path,
                         const Path& base = CurrentDirectory());

}  // namespace filesystem

}  // namespace engine
