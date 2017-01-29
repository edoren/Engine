#pragma once

#include <System/FileSystem/Path.hpp>
#include <Util/Precompiled.hpp>

namespace engine {

namespace filesystem {

ENGINE_API Path CurrentDirectory();

ENGINE_API Path Absolute(const Path& path,
                                    const Path& base = CurrentDirectory());

}  // namespace filesystem

}  // namespace engine
