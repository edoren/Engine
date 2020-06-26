#pragma once

#include <Util/Prerequisites.hpp>

#include <System/String.hpp>
#include <Util/Singleton.hpp>

// On Windows undefine this anoying macro defined by windows.h
#if PLATFORM_IS(PLATFORM_WINDOWS)
    #undef ERROR
#endif

namespace engine {

enum class LogPriority {
    VERBOSE = 1,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL,
};

class ENGINE_API LogManager : public Singleton<LogManager> {
public:
    LogManager();
    LogManager(String appName, String logFile);

    ~LogManager();

    void initialize();

    void shutdown();

    void verbose(const String& tag, const String& message);
    void debug(const String& tag, const String& message);
    void info(const String& tag, const String& message);
    void warning(const String& tag, const String& message);
    void error(const String& tag, const String& message);
    void fatal(const String& tag, const String& message);

    void logMessage(LogPriority priority, const String& tag, const String& message);

    void enableFileLogging(bool enable);
    void enableConsoleLogging(bool enable);

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
    static LogManager& GetInstance();

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
    static LogManager* GetInstancePtr();

private:
    String m_appName;
    String m_logFile;
    bool m_fileLoggingEnable;
    bool m_consoleLoggingEnable;
};

inline void LogVerbose(const String& tag, const String& message) {
    LogManager::GetInstance().verbose(tag, message);
}

inline void LogDebug(const String& tag, const String& message) {
    LogManager::GetInstance().debug(tag, message);
}

inline void LogInfo(const String& tag, const String& message) {
    LogManager::GetInstance().info(tag, message);
}

inline void LogWarning(const String& tag, const String& message) {
    LogManager::GetInstance().warning(tag, message);
}

inline void LogError(const String& tag, const String& message) {
    LogManager::GetInstance().error(tag, message);
}

inline void LogFatal(const String& tag, const String& message) {
    LogManager::GetInstance().fatal(tag, message);
}

}  // namespace engine
