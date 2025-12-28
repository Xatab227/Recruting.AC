#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <mutex>
#include <functional>
#include "Types.h"

namespace ForensicScanner {

enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error,
    Trigger  // Специальный уровень для триггеров
};

struct LogEntry {
    LogLevel level;
    std::string message;
    std::time_t timestamp;
    std::string category;   // Для группировки (hash, browser, discord)
};

class Logger {
public:
    static Logger& Instance();
    
    // Инициализация
    bool Initialize(const std::string& logFilePath = "scan_log.txt");
    void Shutdown();
    
    // Логирование
    void Log(LogLevel level, const std::string& message, const std::string& category = "");
    void Debug(const std::string& message, const std::string& category = "");
    void Info(const std::string& message, const std::string& category = "");
    void Warning(const std::string& message, const std::string& category = "");
    void Error(const std::string& message, const std::string& category = "");
    
    // Логирование триггеров
    void LogTrigger(const TriggerEvent& trigger);
    
    // Получение логов для UI
    const std::vector<LogEntry>& GetLogs() const { return m_logs; }
    std::vector<LogEntry> GetLogsByCategory(const std::string& category) const;
    std::vector<LogEntry> GetLogsByLevel(LogLevel level) const;
    
    // Очистка логов
    void ClearLogs();
    
    // Callback для обновления UI при новом логе
    void SetLogCallback(std::function<void(const LogEntry&)> callback);
    
    // Форматирование
    static std::string LogLevelToString(LogLevel level);
    static std::string FormatLogEntry(const LogEntry& entry);

private:
    Logger() = default;
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    void WriteToFile(const LogEntry& entry);
    
    std::ofstream m_logFile;
    std::vector<LogEntry> m_logs;
    mutable std::mutex m_mutex;
    std::function<void(const LogEntry&)> m_callback;
    bool m_initialized = false;
};

} // namespace ForensicScanner
