#pragma once

#include <Util/Prerequisites.hpp>

#include <System/String.hpp>

namespace engine {

namespace filesystem {

////////////////////////////////////////////////////////////
/// \brief Get the OS specific path separator
///
/// \return The character that represents the path separator
///
////////////////////////////////////////////////////////////
ENGINE_API char8 GetSeparator();

////////////////////////////////////////////////////////////
/// \brief Get the directory that contains the executable
///        in which this function reside.
///
/// \return String containing the executable directory
///
////////////////////////////////////////////////////////////
ENGINE_API String ExecutableDirectory();

////////////////////////////////////////////////////////////
/// \brief Return a string representing the current working
///        directory.
///
/// \return String containing the current working directory
///
////////////////////////////////////////////////////////////
ENGINE_API String CurrentWorkingDirectory();

////////////////////////////////////////////////////////////
/// \brief Return the absolute path
///
/// \return String containing the absolute path
///
////////////////////////////////////////////////////////////
ENGINE_API String AbsolutePath(const String& path);

////////////////////////////////////////////////////////////
/// \brief Normalize a pathname by collapsing redundant
///        separators and up-level references so that A//B,
///        A/B/, A/./B and A/foo/../B all become A/B
///
/// \return String containing the normalized path
///
////////////////////////////////////////////////////////////
ENGINE_API String NormalizePath(const String& path);

////////////////////////////////////////////////////////////
/// \brief Check if a path is absolute
///
/// \return true if the path is absolute, false otherwise
///
////////////////////////////////////////////////////////////
ENGINE_API bool IsAbsolute(const String& path);

////////////////////////////////////////////////////////////
/// \brief Join two path components. The return value is
///        the concatenation of the paths with a the
///        spacific OS Separator between them.
///
/// \return String concatenating the two paths components
///
////////////////////////////////////////////////////////////
ENGINE_API String Join(const String& left, const String& right);

////////////////////////////////////////////////////////////
/// \brief Variadic version of the Join function that
///        accepts multiple path components as arguments.
///
/// \return String concatenating all the provided path
///         components
///
////////////////////////////////////////////////////////////
template <typename... TPaths>
String Join(const String& left, const String& right, TPaths... Paths) {
    return Join(Join(left, right), Paths...);
}

}  // namespace filesystem

}  // namespace engine
