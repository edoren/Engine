#include <System/LogManager.hpp>

#include <System/String.hpp>
#include <System/StringView.hpp>

#include <SDL2.h>

#include <utility>

#if PLATFORM_IS(PLATFORM_ANDROID)
    #include <android/log.h>
#endif

namespace engine {

namespace {

#if PLATFORM_IS(PLATFORM_WINDOWS)
const char* const sLineEnding = "\r\n";
#else
const char* const sLineEnding = "\n";
#endif

#if PLATFORM_IS(PLATFORM_ANDROID)
android_LogPriority sAndroidLogPriorities[] = {ANDROID_LOG_UNKNOWN, ANDROID_LOG_VERBOSE, ANDROID_LOG_DEBUG,
                                               ANDROID_LOG_INFO,    ANDROID_LOG_WARN,    ANDROID_LOG_ERROR,
                                               ANDROID_LOG_FATAL};
#endif

const char* sLogPriorityNames[] = {nullptr, "VERBOSE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"};

String DefaultLogCallback(LogPriority priority, const StringView& tag, const StringView& message) {
    const char* priorityName = sLogPriorityNames[static_cast<int>(priority)];

    // Get the current system hour
    std::time_t t = std::time(nullptr);
    std::tm* tm = std::localtime(&t);

    // Format the log message
    fmt::string_view tagView(tag.getData(), tag.getSize());
    fmt::string_view messageView(message.getData(), message.getSize());
    return "[{:02d}:{:02d}:{:02d}] [{}/{}] : {}"_format(tm->tm_hour, tm->tm_min, tm->tm_sec, tagView,
                                                        priorityName, messageView);
}

}  // namespace

LogManager::LogManager()
      : m_appName("Engine"),
        m_logFile("engine.log"),
        m_fileLoggingEnable(true),
        m_consoleLoggingEnable(true) {}

LogManager::LogManager(String appName, String logFile)
      : m_appName(std::move(appName)),
        m_logFile(std::move(logFile)),
        m_fileLoggingEnable(true),
        m_consoleLoggingEnable(true) {}

LogManager::~LogManager() = default;

void LogManager::initialize() {}

void LogManager::shutdown() {}

void LogManager::verbose(const StringView& tag, const StringView& message) {
    logMessage(LogPriority::VERBOSE, tag, message);
}

void LogManager::debug(const StringView& tag, const StringView& message) {
    logMessage(LogPriority::DEBUG, tag, message);
}

void LogManager::info(const StringView& tag, const StringView& message) {
    logMessage(LogPriority::INFO, tag, message);
}

void LogManager::warning(const StringView& tag, const StringView& message) {
    logMessage(LogPriority::WARN, tag, message);
}

void LogManager::error(const StringView& tag, const StringView& message) {
    logMessage(LogPriority::ERROR, tag, message);
}

void LogManager::fatal(const StringView& tag, const StringView& message) {
    logMessage(LogPriority::FATAL, tag, message);
}

void LogManager::logMessage(LogPriority priority, const StringView& tag, const StringView& message) {
    if (!m_fileLoggingEnable && !m_consoleLoggingEnable) {
        return;
    }

#ifndef ENGINE_DEBUG
    if (priority == LogPriority::DEBUG) {
        return;
    }
#endif

    String logMessage = DefaultLogCallback(priority, tag, message);

    // Write the log to the file
    if (m_fileLoggingEnable) {
        if (SDL_RWops* pfile = SDL_RWFromFile(m_logFile.getData(), "ab")) {
            const auto& str = logMessage.toUtf8();
            SDL_RWwrite(pfile, str.data(), 1, str.size());
            SDL_RWwrite(pfile, sLineEnding, 1, strlen(sLineEnding));
            SDL_RWclose(pfile);
        }
    }

    // Write the log to console
    if (m_consoleLoggingEnable) {
#if PLATFORM_IS(PLATFORM_ANDROID)
        __android_log_write(sAndroidLogPriorities[static_cast<int>(priority)], m_appName.getData(),
                            log_message.getData());
#else
        fputs(logMessage.getData(), stdout);
        fputs("\n", stdout);
#endif
    }

    if (priority == LogPriority::FATAL) {
        std::exit(1);  // TMP
    }
}

void LogManager::enableFileLogging(bool enable) {
    m_fileLoggingEnable = enable;
}

void LogManager::enableConsoleLogging(bool enable) {
    m_consoleLoggingEnable = enable;
}

}  // namespace engine
