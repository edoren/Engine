#include <System/FileSystem/Path.hpp>

namespace engine {

namespace filesystem {

#if PLATFORM == PLATFORM_WINDOWS
const Path::value_type Path::Separator = '\\';
#else
const Path::value_type Path::Separator = '/';
#endif

Path::Path() : path_() {}

Path::Path(const char* p) : Path(String(p)) {}

Path::Path(const String& p)
      : path_(external::fs::u8path(p.Begin(), p.End())) {}

Path::Path(external::fs::path&& p) : path_(std::move(p)) {}

Path& Path::Append(const Path& p) {
    path_ /= p.path_;
    return *this;
}

Path Path::ParentPath() const {
    return Path(path_.parent_path());
}

Path& Path::ReplaceExtension(const Path& replacement) {
    path_.replace_extension(replacement.path_);
    return *this;
}

String Path::Str() const {
    return path_.generic_u8string();
}

Path& Path::operator/=(const Path& p) {
    return Append(p);
}

Path operator/(const Path& lhs, const Path& rhs) {
    Path ret(lhs);
    ret /= rhs;
    return ret;
}

std::ostream& operator<<(std::ostream& os, const Path& p) {
    os << p.path_;
    return os;
}

}  // namespace filesystem

}  // namespace engine
