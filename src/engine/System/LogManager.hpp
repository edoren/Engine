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

    /**
     * @brief Override standard Singleton retrieval.
     *
     * @remarks Why do we do this? Well, it's because the Singleton
     *          implementation is in a .hpp file, which means it gets
     *          compiled into anybody who includes it. This is needed
     *          for the Singleton template to work, but we actually
     *          only compiled into the implementation of the class
     *          based on the Singleton, not all of them. If we don't
     *          change this, we get link errors when trying to use the
     *          Singleton-based class from an outside dll.
     *
     * @par This method just delegates to the template version anyway,
     *      but the implementation stays in this single compilation unit,
     *      preventing link errors.
     */
    static LogManager& GetInstance();

    /**
     * @brief Override standard Singleton retrieval.
     *
     * @remarks Why do we do this? Well, it's because the Singleton
     *          implementation is in a .hpp file, which means it gets
     *          compiled into anybody who includes it. This is needed
     *          for the Singleton template to work, but we actually
     *          only compiled into the implementation of the class
     *          based on the Singleton, not all of them. If we don't
     *          change this, we get link errors when trying to use the
     *          Singleton-based class from an outside dll.
     *
     * @par This method just delegates to the template version anyway,
     *      but the implementation stays in this single compilation unit,
     *      preventing link errors.
     */
    static LogManager* GetInstancePtr();

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
    auto internalStringView = fmt::string_view(message.getData(), message.getSize());
    LogVerbose(tag, String(std::move(fmt::format(internalStringView, std::forward<Args>(args)...))));
}

inline void LogDebug(const StringView& tag, const StringView& message) {
    LogManager::GetInstance().debug(tag, message);
}

template <typename... Args>
inline void LogDebug(const StringView& tag, const StringView& message, Args&&... args) {
    auto internalStringView = fmt::string_view(message.getData(), message.getSize());
    LogDebug(tag, String(std::move(fmt::format(internalStringView, std::forward<Args>(args)...))));
}

inline void LogInfo(const StringView& tag, const StringView& message) {
    LogManager::GetInstance().info(tag, message);
}

template <typename... Args>
inline void LogInfo(const StringView& tag, const StringView& message, Args&&... args) {
    auto internalStringView = fmt::string_view(message.getData(), message.getSize());
    LogInfo(tag, String(std::move(fmt::format(internalStringView, std::forward<Args>(args)...))));
}

inline void LogWarning(const StringView& tag, const StringView& message) {
    LogManager::GetInstance().warning(tag, message);
}

template <typename... Args>
inline void LogWarning(const StringView& tag, const StringView& message, Args&&... args) {
    auto internalStringView = fmt::string_view(message.getData(), message.getSize());
    LogWarning(tag, String(std::move(fmt::format(internalStringView, std::forward<Args>(args)...))));
}

inline void LogError(const StringView& tag, const StringView& message) {
    LogManager::GetInstance().error(tag, message);
}

template <typename... Args>
inline void LogError(const StringView& tag, const StringView& message, Args&&... args) {
    auto internalStringView = fmt::string_view(message.getData(), message.getSize());
    LogError(tag, String(std::move(fmt::format(internalStringView, std::forward<Args>(args)...))));
}

inline void LogFatal(const StringView& tag, const StringView& message) {
    LogManager::GetInstance().fatal(tag, message);
}

template <typename... Args>
inline void LogFatal(const StringView& tag, const StringView& message, Args&&... args) {
    auto internalStringView = fmt::string_view(message.getData(), message.getSize());
    LogFatal(tag, String(std::move(fmt::format(internalStringView, std::forward<Args>(args)...))));
}

}  // namespace engine
