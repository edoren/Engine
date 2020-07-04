#pragma once

#include <Util/Prerequisites.hpp>

#include <System/String.hpp>
#include <System/StringView.hpp>
#include <Util/Container/Vector.hpp>
#include <Util/Singleton.hpp>

namespace engine {

/**
 * @brief Class to manage file system of the current OS
 */
class ENGINE_API FileSystem : public Singleton<FileSystem> {
public:
    FileSystem();

    ~FileSystem();

    void initialize();

    void shutdown();

    /**
     * @brief Checks if a file exist
     *
     * @return true if the file exist, false otherwise
     */
    bool fileExists(const StringView& filename) const;

    /**
     * @brief Load an UTF-8 encoded file to the memory
     *
     * @param filename The file to load the data from
     * @param dest A string to store de file text
     * @return true if the file could be loaded, false
     *         otherwise
     */
    bool loadFileData(const String& filename, String* dest) const;

    /**
     * @brief Load a file to the memory
     *
     * @param filename The file to load the data from
     * @param dest A vector to store de file bytes
     * @return true if the file could be loaded, false
     *         otherwise
     */
    bool loadFileData(const String& filename, Vector<byte>* dest) const;

    /**
     * @brief Get the OS specific path separator
     *
     * @return The character that represents the path separator
     */
    char8 getOsSeparator() const;

    /**
     * @brief Get the directory that contains the executable
     *        in which this function reside.
     *
     * @return String containing the executable directory
     */
    const String& executableDirectory() const;

    /**
     * @brief Return a string representing the current working
     *        directory.
     *
     * @return String containing the current working directory
     */
    String currentWorkingDirectory() const;

    /**
     * @brief Return the absolute path
     *
     * @param path The relative path to make absolute
     * @return String containing the absolute path
     */
    String absolutePath(const String& path) const;

    /**
     * @brief Normalize a pathname by collapsing redundant
     *        separators and up-level references so that A//B,
     *        A/B/, A/./B and A/foo/../B all become A/B
     *
     * On Windows this function also change any '/' character
     * to '\'
     *
     * @param path The path to normalize
     * @return String containing the normalized path
     */
    String normalizePath(const String& path) const;

    /**
     * @brief Check if a path is absolute
     *
     * @param path The path to check
     * @return true if the path is absolute, false otherwise
     */
    bool isAbsolutePath(const StringView& path) const;

    /**
     * @brief Join two path components. The return value is
     *        the concatenation of the paths with a the
     *        spacific OS Separator between them.
     *
     * @param left The first path to join
     * @param right The second path to join
     * @return String concatenating the two paths components
     */
    String join(const StringView& left, const StringView& right) const;

    /**
     * @brief Variadic version of the Join function that
     *        accepts multiple path components as arguments.
     *
     * @tparam Args Variadic arguments
     * @param left The first path to join
     * @param right The second path to join
     * @param paths Additional paths to join
     * @return String concatenating all the provided path components
     */
    template <typename... Args>
    String join(const StringView& left, const StringView& right, Args... paths) const {
        return join(join(left, right), paths...);
    }

    /**
     * @brief Change the search paths used to open files
     *
     * @param searchPaths Vector of new search paths
     */
    void setSearchPaths(Vector<String> searchPaths);

    /**
     * @brief Obtain the serch paths used to open files
     *
     * @return A constant reference to the currently used search
     *         paths
     */
    const Vector<String>& getSearchPaths() const;

    /**
     * @brief Add a path to the search paths used to open files
     *
     * @param path That is going to added to the search paths
     * @return A constant reference to the currently used search
     *         paths
     */
    void addSearchPath(const String& path);

private:
    Vector<String> m_searchPaths;
};

template <>
FileSystem* Singleton<FileSystem>::sInstance;

}  // namespace engine
