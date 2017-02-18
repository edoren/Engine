#include <System/FileSystem/Dependencies.hpp>
#include <System/FileSystem/Operations.hpp>

namespace engine {

namespace filesystem {

Path Absolute(const Path& path, const Path& base) {
    return Path(external::fs::absolute(path.path_, base.path_));
}

Path CurrentDirectory() {
    return Path(external::fs::current_path());
}

}  // namespace filesystem

}  // namespace engine
