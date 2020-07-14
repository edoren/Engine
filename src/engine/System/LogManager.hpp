#pragma once

#include <Util/Prerequisites.hpp>

#include <System/String.hpp>
#include <System/StringFormat.hpp>
#include <System/StringView.hpp>
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

    void verbose(const StringView& tag, const StringView& message);
    void debug(const StringView& tag, const StringView& message);
    void info(const StringView& tag, const StringView& message);
    void warning(const StringView& tag, const StringView& message);
    void error(const StringView& tag, const StringView& message);
    void fatal(const StringView& tag, const StringView& message);

    void logMessage(LogPriority priority, const StringView& tag, const StringView& message);

    void enableFileLogging(bool enable);
    void enableConsoleLogging(bool enable);

private:
    String m_appName;
    String m_logFile;
    bool m_fileLoggingEnable;
    bool m_consoleLoggingEnable;
};

inline void LogVerbose(const StringView& tag, const StringView& message) {
    LogManager::GetInstance().verbose(tag, message);
}

template <typename... Args>
inline void LogVerbose(const StringView& tag, const StringView& message, Args&&... args) {
    auto internalStringView = fmt::string_view(reinterpret_cast<const char*>(message.getData()), message.getSize());
    auto formated = fmt::format(internalStringView, std::forward<Args>(args)...);
    LogVerbose(tag, StringView(formated.data()));
}

inline void LogDebug(const StringView& tag, const StringView& message) {
    LogManager::GetInstance().debug(tag, message);
}

template <typename... Args>
inline void LogDebug(const StringView& tag, const StringView& message, Args&&... args) {
    auto internalStringView = fmt::string_view(reinterpret_cast<const char*>(message.getData()), message.getSize());
    auto formated = fmt::format(internalStringView, std::forward<Args>(args)...);
    LogDebug(tag, StringView(formated.data()));
}

inline void LogInfo(const StringView& tag, const StringView& message) {
    LogManager::GetInstance().info(tag, message);
}

template <typename... Args>
inline void LogInfo(const StringView& tag, const StringView& message, Args&&... args) {
    auto internalStringView = fmt::string_view(reinterpret_cast<const char*>(message.getData()), message.getSize());
    auto formated = fmt::format(internalStringView, std::forward<Args>(args)...);
    LogInfo(tag, StringView(formated.data()));
}

inline void LogWarning(const StringView& tag, const StringView& message) {
    LogManager::GetInstance().warning(tag, message);
}

template <typename... Args>
inline void LogWarning(const StringView& tag, const StringView& message, Args&&... args) {
    auto internalStringView = fmt::string_view(reinterpret_cast<const char*>(message.getData()), message.getSize());
    auto formated = fmt::format(internalStringView, std::forward<Args>(args)...);
    LogWarning(tag, StringView(formated.data()));
}

inline void LogError(const StringView& tag, const StringView& message) {
    LogManager::GetInstance().error(tag, message);
}

template <typename... Args>
inline void LogError(const StringView& tag, const StringView& message, Args&&... args) {
    auto internalStringView = fmt::string_view(reinterpret_cast<const char*>(message.getData()), message.getSize());
    auto formated = fmt::format(internalStringView, std::forward<Args>(args)...);
    LogError(tag, StringView(formated.data()));
}

inline void LogFatal(const StringView& tag, const StringView& message) {
    LogManager::GetInstance().fatal(tag, message);
}

template <typename... Args>
inline void LogFatal(const StringView& tag, const StringView& message, Args&&... args) {
    auto internalStringView = fmt::string_view(reinterpret_cast<const char*>(message.getData()), message.getSize());
    auto formated = fmt::format(internalStringView, std::forward<Args>(args)...);
    LogFatal(tag, StringView(formated.data()));
}

}  // namespace engine
