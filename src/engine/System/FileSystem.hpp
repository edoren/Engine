#pragma once

#include <Util/Prerequisites.hpp>

#include <System/String.hpp>

namespace engine {

class ENGINE_API FileSystem : public Singleton<FileSystem> {
public:
    FileSystem();

    ~FileSystem();

    void Initialize();

    void Shutdown();

    ////////////////////////////////////////////////////////////
    /// @brief Checks if a file exist
    ///
    /// @return true if the file exist, false otherwise
    ////////////////////////////////////////////////////////////
    bool FileExists(const String& filename) const;

    ////////////////////////////////////////////////////////////
    /// @brief Load a file to the memory
    ///
    /// @return true if the file could be loaded, false
    ///         otherwise
    ////////////////////////////////////////////////////////////
    bool LoadFileData(const String& filename, String* dest) const;

    ////////////////////////////////////////////////////////////
    /// @brief Load a file to the memory
    ///
    /// @return true if the file could be loaded, false
    ///         otherwise
    ////////////////////////////////////////////////////////////
    bool LoadFileData(const String& filename, std::vector<byte>* dest) const;

    ////////////////////////////////////////////////////////////
    /// @brief Get the OS specific path separator
    ///
    /// @return The character that represents the path separator
    ////////////////////////////////////////////////////////////
    char8 GetOsSeparator() const;

    ////////////////////////////////////////////////////////////
    /// @brief Get the directory that contains the executable
    ///        in which this function reside.
    ///
    /// @return String containing the executable directory
    ////////////////////////////////////////////////////////////
    String ExecutableDirectory() const;

    ////////////////////////////////////////////////////////////
    /// @brief Return a string representing the current working
    ///        directory.
    ///
    /// @return String containing the current working directory
    ////////////////////////////////////////////////////////////
    String CurrentWorkingDirectory() const;

    ////////////////////////////////////////////////////////////
    /// @brief Return the absolute path
    ///
    /// @return String containing the absolute path
    ////////////////////////////////////////////////////////////
    String AbsolutePath(const String& path) const;

    ////////////////////////////////////////////////////////////
    /// @brief Normalize a pathname by collapsing redundant
    ///        separators and up-level references so that A//B,
    ///        A/B/, A/./B and A/foo/../B all become A/B
    ///
    /// On Windows this function also change any '/' character
    /// to '\'
    ///
    /// @return String containing the normalized path
    ////////////////////////////////////////////////////////////
    String NormalizePath(const String& path) const;

    ////////////////////////////////////////////////////////////
    /// @brief Check if a path is absolute
    ///
    /// @return true if the path is absolute, false otherwise
    ////////////////////////////////////////////////////////////
    bool IsAbsolutePath(const String& path) const;

    ////////////////////////////////////////////////////////////
    /// @brief Join two path components. The return value is
    ///        the concatenation of the paths with a the
    ///        spacific OS Separator between them.
    ///
    /// @return String concatenating the two paths components
    ////////////////////////////////////////////////////////////
    String Join(const String& left, const String& right) const;

    ////////////////////////////////////////////////////////////
    /// @brief Variadic version of the Join function that
    ///        accepts multiple path components as arguments.
    ///
    /// @return String concatenating all the provided path
    ///         components
    ////////////////////////////////////////////////////////////
    template <typename... Args>
    String Join(const String& left, const String& right, Args... Paths) const {
        return Join(Join(left, right), Paths...);
    }

    ////////////////////////////////////////////////////////////
    /// @brief Change the search paths used to open files
    ///
    /// @param[in] search_paths Vector of new search paths
    ////////////////////////////////////////////////////////////
    void SetSearchPaths(std::vector<String> search_paths);

    ////////////////////////////////////////////////////////////
    /// @brief Obtain the serch paths used to open files
    ///
    /// @return A constant reference to the currently used search
    ///         paths
    ////////////////////////////////////////////////////////////
    const std::vector<String>& GetSearchPaths() const;

    ////////////////////////////////////////////////////////////
    /// @brief Add a path to the search paths used to open files
    ///
    /// @param[in] path That is going to added to the search paths
    ///
    /// @return A constant reference to the currently used search
    ///         paths
    ////////////////////////////////////////////////////////////
    void AddSearchPath(const String& path);

    ////////////////////////////////////////////////////////////
    /// @brief Override standard Singleton retrieval.
    ///
    /// @remarks Why do we do this? Well, it's because the Singleton
    ///          implementation is in a .hpp file, which means it gets
    ///          compiled into anybody who includes it. This is needed
    ///          for the Singleton template to work, but we actually
    ///          only compiled into the implementation of the class
    ///          based on the Singleton, not all of them. If we don't
    ///          change this, we get link errors when trying to use the
    ///          Singleton-based class from an outside dll.
    ///
    /// @par This method just delegates to the template version anyway,
    ///      but the implementation stays in this single compilation unit,
    ///      preventing link errors.
    ////////////////////////////////////////////////////////////
    static FileSystem& GetInstance();

    ////////////////////////////////////////////////////////////
    /// @brief Override standard Singleton retrieval.
    ///
    /// @remarks Why do we do this? Well, it's because the Singleton
    ///          implementation is in a .hpp file, which means it gets
    ///          compiled into anybody who includes it. This is needed
    ///          for the Singleton template to work, but we actually
    ///          only compiled into the implementation of the class
    ///          based on the Singleton, not all of them. If we don't
    ///          change this, we get link errors when trying to use the
    ///          Singleton-based class from an outside dll.
    ///
    /// @par This method just delegates to the template version anyway,
    ///      but the implementation stays in this single compilation unit,
    ///      preventing link errors.
    ////////////////////////////////////////////////////////////
    static FileSystem* GetInstancePtr();

private:
    std::vector<String> m_search_paths;
};

}  // namespace engine
