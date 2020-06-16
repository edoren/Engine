#include <System/LogManager.hpp>
#include <System/StringFormat.hpp>

#include <SDL2.h>

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

String DefaultLogCallback(LogPriority priority, const String& tag, const String& message) {
    const char* priority_name = sLogPriorityNames[static_cast<int>(priority)];

    // Get the current system hour
    std::time_t t = std::time(nullptr);
    std::tm* tm = std::localtime(&t);

    // Format the log message
    return "[{:02d}:{:02d}:{:02d}] [{}/{}] : {}"_format(tm->tm_hour, tm->tm_min, tm->tm_sec, tag.getData(),
                                                        priority_name, message.getData());
}

}  // namespace

template <>
LogManager* Singleton<LogManager>::sInstance = nullptr;

LogManager& LogManager::GetInstance() {
    return Singleton<LogManager>::GetInstance();
}

LogManager* LogManager::GetInstancePtr() {
    return Singleton<LogManager>::GetInstancePtr();
}

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

void LogManager::verbose(const String& tag, const String& message) {
    logMessage(LogPriority::VERBOSE, tag, message);
}

void LogManager::debug(const String& tag, const String& message) {
    logMessage(LogPriority::DEBUG, tag, message);
}

void LogManager::info(const String& tag, const String& message) {
    logMessage(LogPriority::INFO, tag, message);
}

void LogManager::warning(const String& tag, const String& message) {
    logMessage(LogPriority::WARN, tag, message);
}

void LogManager::error(const String& tag, const String& message) {
    logMessage(LogPriority::ERROR, tag, message);
}

void LogManager::fatal(const String& tag, const String& message) {
    logMessage(LogPriority::FATAL, tag, message);
    std::exit(1);  // TMP
}

void LogManager::logMessage(LogPriority priority, const String& tag, const String& message) {
#ifndef ENGINE_DEBUG
    if (priority == LogPriority::DEBUG) return;
#endif

    String log_message = DefaultLogCallback(priority, tag, message);

    // Write the log to the file
    if (m_fileLoggingEnable) {
        if (SDL_RWops* pfile = SDL_RWFromFile(m_logFile.getData(), "ab")) {
            const std::string& str = log_message.toUtf8();
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
        fputs(log_message.getData(), stdout);
        fputs("\n", stdout);
#endif
    }
}

void LogManager::enableFileLogging(bool enable) {
    m_fileLoggingEnable = enable;
}

void LogManager::enableConsoleLogging(bool enable) {
    m_consoleLoggingEnable = enable;
}

}  // namespace engine
