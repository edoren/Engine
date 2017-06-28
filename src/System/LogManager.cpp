#include <System/LogManager.hpp>
#include <System/StringFormat.hpp>

#include <SDL.h>

#if PLATFORM_IS(PLATFORM_ANDROID)
#include <android/log.h>
#endif

namespace engine {

namespace {

#if PLATFORM_IS(PLATFORM_ANDROID)
android_LogPriority s_android_log_priorities[] = {
    ANDROID_LOG_UNKNOWN, ANDROID_LOG_VERBOSE, ANDROID_LOG_DEBUG,
    ANDROID_LOG_INFO,    ANDROID_LOG_WARN,    ANDROID_LOG_ERROR,
    ANDROID_LOG_FATAL};
#endif

const char* s_log_priority_names[] = {NULL,   "VERBOSE", "DEBUG", "INFO",
                                      "WARN", "ERROR",   "FATAL"};

String DefaultLogCallback(LogPriority priority, const String& tag,
                          const String& message) {
    const char* priority_name =
        s_log_priority_names[static_cast<int>(priority)];

    // Get the current system hour
    std::time_t t = std::time(nullptr);
    std::tm* tm = std::localtime(&t);

    // Format the log message
    return "[{:02d}:{:02d}:{:02d}] [{}/{}] : {}"_format(
        tm->tm_hour, tm->tm_min, tm->tm_sec, tag.GetData(), priority_name,
        message.GetData());
}

}  // namespace

template <>
LogManager* Singleton<LogManager>::s_instance = nullptr;

LogManager& LogManager::GetInstance() {
    assert(s_instance);
    return (*s_instance);
}

LogManager* LogManager::GetInstancePtr() {
    return s_instance;
}

LogManager::LogManager()
      : m_app_name("Engine"),
        m_log_file("engine.log"),
        m_file_logging_enable(true),
        m_console_logging_enable(true) {}

LogManager::LogManager(const String& app_name, const String& log_file)
      : m_app_name(app_name),
        m_log_file(log_file),
        m_file_logging_enable(true),
        m_console_logging_enable(true) {}

LogManager::~LogManager() {}

void LogManager::Verbose(const String& tag, const String& message) {
    LogMessage(LogPriority::VERBOSE, tag, message);
}

void LogManager::Debug(const String& tag, const String& message) {
    LogMessage(LogPriority::DEBUG, tag, message);
}

void LogManager::Info(const String& tag, const String& message) {
    LogMessage(LogPriority::INFO, tag, message);
}

void LogManager::Warning(const String& tag, const String& message) {
    LogMessage(LogPriority::WARN, tag, message);
}

void LogManager::Error(const String& tag, const String& message) {
    LogMessage(LogPriority::ERROR, tag, message);
}

void LogManager::Fatal(const String& tag, const String& message) {
    LogMessage(LogPriority::FATAL, tag, message);
    std::exit(1);  // TMP
}

void LogManager::LogMessage(LogPriority priority, const String& tag,
                            const String& message) {
#ifndef ENGINE_DEBUG
    if (priority == LogPriority::DEBUG) return;
#endif

    String log_message = DefaultLogCallback(priority, tag, message);

    // Write the log to the file
    if (m_file_logging_enable) {
        SDL_RWops* pfile = nullptr;
        pfile = SDL_RWFromFile(m_log_file.GetData(), "ab");
        const std::string& str = log_message.ToUtf8();
        SDL_RWwrite(pfile, str.data(), 1, str.size());
        SDL_RWwrite(pfile, "\n", 1, 1);
        SDL_RWclose(pfile);
    }

    // Write the log to console
    if (m_console_logging_enable) {
#if PLATFORM_IS(PLATFORM_ANDROID)
        __android_log_write(
            s_android_log_priorities[static_cast<int>(priority)],
            m_app_name.GetData(), log_message.GetData());
#else
        fputs(log_message.GetData(), stdout);
        fputs("\n", stdout);
#endif
    }
}

void LogManager::EnableFileLogging(bool enable) {
    m_file_logging_enable = enable;
}

void LogManager::EnableConsoleLogging(bool enable) {
    m_console_logging_enable = enable;
}

}  // namespace engine
