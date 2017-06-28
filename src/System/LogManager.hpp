#pragma once

#include <Util/Prerequisites.hpp>

#include <System/String.hpp>

namespace engine {

enum class LogPriority : int {
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
    LogManager(const String& app_name, const String& log_file);

    ~LogManager();

    void Verbose(const String& tag, const String& message);
    void Debug(const String& tag, const String& message);
    void Info(const String& tag, const String& message);
    void Warning(const String& tag, const String& message);
    void Error(const String& tag, const String& message);
    void Fatal(const String& tag, const String& message);

    void LogMessage(LogPriority priority, const String& tag,
                    const String& message);

    void EnableFileLogging(bool enable);
    void EnableConsoleLogging(bool enable);

    // Override standard Singleton retrieval.
    //     @remarks
    //         Why do we do this? Well, it's because the Singleton
    //         implementation is in a .h file, which means it gets compiled
    //         into anybody who includes it. This is needed for the
    //         Singleton template to work, but we actually only want it
    //         compiled into the implementation of the class based on the
    //         Singleton, not all of them. If we don't change this, we get
    //         link errors when trying to use the Singleton-based class from
    //         an outside dll.
    //     @par
    //         This method just delegates to the template version anyway,
    //         but the implementation stays in this single compilation unit,
    //         preventing link errors.
    static LogManager& GetInstance();

    // Override standard Singleton retrieval.
    //     @remarks
    //         Why do we do this? Well, it's because the Singleton
    //         implementation is in a .h file, which means it gets compiled
    //         into anybody who includes it. This is needed for the
    //         Singleton template to work, but we actually only want it
    //         compiled into the implementation of the class based on the
    //         Singleton, not all of them. If we don't change this, we get
    //         link errors when trying to use the Singleton-based class from
    //         an outside dll.
    //     @par
    //         This method just delegates to the template version anyway,
    //         but the implementation stays in this single compilation unit,
    //         preventing link errors.
    static LogManager* GetInstancePtr();

private:
    String m_app_name;
    String m_log_file;
    bool m_file_logging_enable;
    bool m_console_logging_enable;
};

inline void LogVerbose(const String& tag, const String& message) {
    LogManager::GetInstance().Verbose(tag, message);
}

inline void LogDebug(const String& tag, const String& message) {
    LogManager::GetInstance().Debug(tag, message);
}

inline void LogInfo(const String& tag, const String& message) {
    LogManager::GetInstance().Info(tag, message);
}

inline void LogWarning(const String& tag, const String& message) {
    LogManager::GetInstance().Warning(tag, message);
}

inline void LogError(const String& tag, const String& message) {
    LogManager::GetInstance().Error(tag, message);
}

inline void LogFatal(const String& tag, const String& message) {
    LogManager::GetInstance().Fatal(tag, message);
}

}  // namespace engine
