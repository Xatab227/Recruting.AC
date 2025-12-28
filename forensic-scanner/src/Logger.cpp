/**
 * @file Logger.cpp
 * @brief Реализация модуля логирования
 */

#include "../include/Logger.h"
#include <iomanip>
#include <sstream>
#include <locale>
#include <codecvt>

namespace ForensicScanner {

Logger::Logger() : m_initialized(false) {
}

Logger::~Logger() {
    shutdown();
}

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

bool Logger::initialize(const std::wstring& logFilePath) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_initialized) {
        return true;
    }
    
    m_logFile.open(logFilePath, std::ios::out | std::ios::trunc);
    if (!m_logFile.is_open()) {
        return false;
    }
    
    // Устанавливаем UTF-8 для файла
    m_logFile.imbue(std::locale(m_logFile.getloc(), 
                    new std::codecvt_utf8<wchar_t, 0x10ffff, std::generate_header>));
    
    m_initialized = true;
    
    // Записываем заголовок
    writeToFile(L"========================================");
    writeToFile(L"Форензик-сканер - Лог сканирования");
    writeToFile(L"Начало: " + formatTime(std::time(nullptr)));
    writeToFile(L"========================================\n");
    
    return true;
}

void Logger::shutdown() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_initialized && m_logFile.is_open()) {
        writeToFile(L"\n========================================");
        writeToFile(L"Окончание: " + formatTime(std::time(nullptr)));
        writeToFile(L"========================================");
        m_logFile.close();
    }
    
    m_initialized = false;
}

void Logger::addEntry(const LogEntry& entry) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    LogEntry newEntry = entry;
    newEntry.timestamp = std::time(nullptr);
    m_entries.push_back(newEntry);
    
    // Записываем в файл
    std::wostringstream oss;
    oss << L"[" << formatTime(newEntry.timestamp) << L"] ";
    oss << L"[" << categoryToString(newEntry.category) << L"] ";
    oss << L"[" << triggerTypeToString(newEntry.triggerType) << L"] ";
    oss << L"Источник: " << newEntry.source << L" | ";
    oss << L"Значение: " << newEntry.matchedValue << L" | ";
    oss << L"Риск: " << newEntry.riskWeight << L"%";
    
    if (!newEntry.details.empty()) {
        oss << L" | Детали: " << newEntry.details;
    }
    
    writeToFile(oss.str());
}

void Logger::info(LogCategory category, const std::wstring& message) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::wostringstream oss;
    oss << L"[" << formatTime(std::time(nullptr)) << L"] ";
    oss << L"[ИНФО] [" << categoryToString(category) << L"] ";
    oss << message;
    
    writeToFile(oss.str());
}

void Logger::warning(LogCategory category, const std::wstring& message) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::wostringstream oss;
    oss << L"[" << formatTime(std::time(nullptr)) << L"] ";
    oss << L"[ПРЕДУПРЕЖДЕНИЕ] [" << categoryToString(category) << L"] ";
    oss << message;
    
    writeToFile(oss.str());
}

void Logger::error(LogCategory category, const std::wstring& message) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::wostringstream oss;
    oss << L"[" << formatTime(std::time(nullptr)) << L"] ";
    oss << L"[ОШИБКА] [" << categoryToString(category) << L"] ";
    oss << message;
    
    writeToFile(oss.str());
}

const std::vector<LogEntry>& Logger::getEntries() const {
    return m_entries;
}

std::vector<LogEntry> Logger::getEntriesByCategory(LogCategory category) const {
    std::vector<LogEntry> filtered;
    
    for (const auto& entry : m_entries) {
        if (entry.category == category) {
            filtered.push_back(entry);
        }
    }
    
    return filtered;
}

int Logger::getTriggerCount(LogCategory category) const {
    int count = 0;
    
    for (const auto& entry : m_entries) {
        if (entry.category == category) {
            count++;
        }
    }
    
    return count;
}

void Logger::clear() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_entries.clear();
}

void Logger::writeToFile(const std::wstring& message) {
    if (m_logFile.is_open()) {
        m_logFile << message << std::endl;
        m_logFile.flush();
    }
}

std::wstring Logger::formatTime(std::time_t time) const {
    std::tm* tm = std::localtime(&time);
    
    std::wostringstream oss;
    oss << std::setfill(L'0');
    oss << std::setw(2) << tm->tm_mday << L".";
    oss << std::setw(2) << (tm->tm_mon + 1) << L".";
    oss << (tm->tm_year + 1900) << L" ";
    oss << std::setw(2) << tm->tm_hour << L":";
    oss << std::setw(2) << tm->tm_min << L":";
    oss << std::setw(2) << tm->tm_sec;
    
    return oss.str();
}

std::wstring Logger::categoryToString(LogCategory category) {
    switch (category) {
        case LogCategory::HASH:     return L"ХЕШИ";
        case LogCategory::BROWSER:  return L"БРАУЗЕР";
        case LogCategory::DISCORD:  return L"DISCORD";
        case LogCategory::SYSTEM:   return L"СИСТЕМА";
        default:                    return L"НЕИЗВЕСТНО";
    }
}

std::wstring Logger::triggerTypeToString(TriggerType type) {
    switch (type) {
        case TriggerType::HASH_MATCH:       return L"Совпадение хеша";
        case TriggerType::KEYWORD_MATCH:    return L"Ключевое слово";
        case TriggerType::URL_MATCH:        return L"Совпадение URL";
        case TriggerType::DISCORD_SERVER:   return L"Discord сервер";
        case TriggerType::DISCORD_CHANNEL:  return L"Discord канал";
        case TriggerType::DISCORD_KEYWORD:  return L"Discord ключевое слово";
        default:                            return L"Неизвестно";
    }
}

} // namespace ForensicScanner
