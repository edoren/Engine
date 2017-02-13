#include <System/LogManager.hpp>

#include <ctime>

#define MAX_LOG_MESSAGE_LENGTH 512

namespace engine {

namespace {

const char* g_log_priority_names[] = {NULL,   "VERBOSE", "DEBUG", "INFO",
                                      "WARN", "ERROR",   "FATAL"};

String DefaultLogCallback(LogPriority priority, const String& tag,
                          const String& message) {
    const char* priority_name =
        g_log_priority_names[static_cast<int>(priority)];

    // Get the current system hour
    std::time_t t = std::time(nullptr);
    std::tm* tm = std::localtime(&t);

    // Format the log message
    char output[4 * MAX_LOG_MESSAGE_LENGTH];
    sprintf(output, "[%.2d:%.2d:%.2d] [%s/%s] : %s\n", tm->tm_hour, tm->tm_min,
            tm->tm_sec, tag.GetData(), priority_name, message.GetData());

    return String::FromUtf8(output, output + strlen(output));
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

LogManager::LogManager() : m_log_file("engine.log") {
    EnableFileLogging(true);
    EnableConsoleLogging(true);
}

LogManager::LogManager(const String& log_file) : m_log_file(log_file) {}

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
        fprintf(stdout, "%s\n", log_message.GetData());
    }
}

void LogManager::EnableFileLogging(bool enable) {
    m_file_logging_enable = enable;
}

void LogManager::EnableConsoleLogging(bool enable) {
    m_console_logging_enable = enable;
}

}  // namespace engine
