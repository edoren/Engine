#pragma once

#include <Util/Prerequisites.hpp>
#include <System/FileSystem/Dependencies.hpp>  // TMP
#include <System/String.hpp>

namespace engine {

namespace filesystem {

class ENGINE_API Path {
public:
    typedef char value_type;
    typedef std::basic_string<value_type> string_type;

    static const value_type Separator;

    Path();
    Path(const char* p);
    Path(const String& p);
    Path(const Path& other) = default;

private:
    explicit Path(external::fs::path&& p);

public:
    Path& Append(const Path& p);

    Path ParentPath() const;

    Path& ReplaceExtension(const Path& replacement = Path());

    String Str() const;

    Path& operator/=(const Path& p);

    friend ENGINE_API Path Absolute(const Path& path, const Path& base);
    friend ENGINE_API Path CurrentDirectory();

    friend ENGINE_API Path operator/(const Path& lhs, const Path& rhs);
    friend ENGINE_API std::ostream& operator<<(std::ostream& os,
                                               const Path& path);

private:
    external::fs::path m_path;
};

ENGINE_API Path operator/(const Path& lhs, const Path& rhs);

ENGINE_API std::ostream& operator<<(std::ostream& os, const Path& path);

}  // namespace filesystem

}  // namespace engine
