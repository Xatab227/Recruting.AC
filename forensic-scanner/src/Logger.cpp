#include "Logger.h"
#include "Utils.h"
#include <iostream>
#include <sstream>
#include <iomanip>

namespace ForensicScanner {

Logger& Logger::Instance() {
    static Logger instance;
    return instance;
}

Logger::~Logger() {
    Shutdown();
}

bool Logger::Initialize(const std::string& logFilePath) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_initialized) {
        return true;
    }
    
    m_logFile.open(logFilePath, std::ios::out | std::ios::trunc);
    if (!m_logFile.is_open()) {
        std::cerr << "Ошибка: не удалось создать файл лога: " << logFilePath << std::endl;
        return false;
    }
    
    m_initialized = true;
    
    // Записываем заголовок
    m_logFile << "========================================\n";
    m_logFile << "  Forensic Scanner - Лог сканирования\n";
    m_logFile << "  Начало: " << Utils::FormatTimestamp(Utils::GetCurrentTimestamp()) << "\n";
    m_logFile << "========================================\n\n";
    m_logFile.flush();
    
    return true;
}

void Logger::Shutdown() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_logFile.is_open()) {
        m_logFile << "\n========================================\n";
        m_logFile << "  Завершение: " << Utils::FormatTimestamp(Utils::GetCurrentTimestamp()) << "\n";
        m_logFile << "========================================\n";
        m_logFile.close();
    }
    
    m_initialized = false;
}

void Logger::Log(LogLevel level, const std::string& message, const std::string& category) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    LogEntry entry;
    entry.level = level;
    entry.message = message;
    entry.timestamp = Utils::GetCurrentTimestamp();
    entry.category = category;
    
    m_logs.push_back(entry);
    
    // Записываем в файл
    WriteToFile(entry);
    
    // Вызываем callback для UI
    if (m_callback) {
        m_callback(entry);
    }
}

void Logger::Debug(const std::string& message, const std::string& category) {
    Log(LogLevel::Debug, message, category);
}

void Logger::Info(const std::string& message, const std::string& category) {
    Log(LogLevel::Info, message, category);
}

void Logger::Warning(const std::string& message, const std::string& category) {
    Log(LogLevel::Warning, message, category);
}

void Logger::Error(const std::string& message, const std::string& category) {
    Log(LogLevel::Error, message, category);
}

void Logger::LogTrigger(const TriggerEvent& trigger) {
    std::stringstream ss;
    ss << "[ТРИГГЕР] " << TriggerTypeToString(trigger.type) << "\n";
    ss << "  Источник: " << trigger.source << "\n";
    ss << "  Значение: " << trigger.matchedValue << "\n";
    ss << "  Вес: " << trigger.weight << "%";
    
    if (!trigger.context.empty()) {
        ss << "\n  Контекст: " << trigger.context;
    }
    
    std::string category;
    switch (trigger.category) {
        case TriggerCategory::Hash: category = "hash"; break;
        case TriggerCategory::Browser: category = "browser"; break;
        case TriggerCategory::Discord: category = "discord"; break;
    }
    
    Log(LogLevel::Trigger, ss.str(), category);
}

std::vector<LogEntry> Logger::GetLogsByCategory(const std::string& category) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::vector<LogEntry> result;
    for (const auto& entry : m_logs) {
        if (entry.category == category) {
            result.push_back(entry);
        }
    }
    return result;
}

std::vector<LogEntry> Logger::GetLogsByLevel(LogLevel level) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::vector<LogEntry> result;
    for (const auto& entry : m_logs) {
        if (entry.level == level) {
            result.push_back(entry);
        }
    }
    return result;
}

void Logger::ClearLogs() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_logs.clear();
}

void Logger::SetLogCallback(std::function<void(const LogEntry&)> callback) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_callback = callback;
}

std::string Logger::LogLevelToString(LogLevel level) {
    switch (level) {
        case LogLevel::Debug:   return "DEBUG";
        case LogLevel::Info:    return "INFO";
        case LogLevel::Warning: return "WARNING";
        case LogLevel::Error:   return "ERROR";
        case LogLevel::Trigger: return "TRIGGER";
        default:                return "UNKNOWN";
    }
}

std::string Logger::FormatLogEntry(const LogEntry& entry) {
    std::stringstream ss;
    ss << "[" << Utils::FormatTimestamp(entry.timestamp) << "] ";
    ss << "[" << LogLevelToString(entry.level) << "] ";
    if (!entry.category.empty()) {
        ss << "[" << entry.category << "] ";
    }
    ss << entry.message;
    return ss.str();
}

void Logger::WriteToFile(const LogEntry& entry) {
    if (!m_logFile.is_open()) {
        return;
    }
    
    m_logFile << FormatLogEntry(entry) << "\n";
    m_logFile.flush();
}

} // namespace ForensicScanner
