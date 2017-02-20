#include <System/FileSystem/Path.hpp>

namespace engine {

namespace filesystem {

#if PLATFORM == PLATFORM_WINDOWS
const Path::value_type Path::Separator = '\\';
#else
const Path::value_type Path::Separator = '/';
#endif

Path::Path() : m_path() {}

Path::Path(const char* p) : Path(String(p)) {}

Path::Path(const String& p)
      : m_path(external::fs::u8path(p.Begin(), p.End())) {}

Path::Path(external::fs::path&& p) : m_path(std::move(p)) {}

Path& Path::Append(const Path& p) {
    m_path /= p.m_path;
    return *this;
}

Path Path::ParentPath() const {
    return Path(m_path.parent_path());
}

Path& Path::ReplaceExtension(const Path& replacement) {
    m_path.replace_extension(replacement.m_path);
    return *this;
}

String Path::Str() const {
    return m_path.generic_u8string();
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
    os << p.m_path;
    return os;
}

}  // namespace filesystem

}  // namespace engine
