#pragma once

#include <ostream>

#include <Util/Precompiled.hpp>

namespace engine {

namespace filesystem {

class Path {
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

    friend Path Absolute(const Path& path, const Path& base);
    friend Path CurrentDirectory();

    friend Path operator/(const Path& lhs, const Path& rhs);
    friend std::ostream& operator<<(std::ostream& os, const Path& path);

private:
    external::fs::path path_;
};

Path operator/(const Path& lhs, const Path& rhs);

std::ostream& operator<<(std::ostream& os, const Path& path);

}  // namespace filesystem

}  // namespace engine
