#include "Logger.h"
#include <algorithm>
#include <iterator>

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

void Logger::log(LogType type, const std::string& triggerType, 
                 const std::string& source, const std::string& matchedValue, int riskWeight) {
    std::lock_guard<std::mutex> lock(logMutex);
    logs.emplace_back(type, triggerType, source, matchedValue, riskWeight);
}

std::vector<LogEntry> Logger::getHashLogs() const {
    std::lock_guard<std::mutex> lock(logMutex);
    std::vector<LogEntry> result;
    std::copy_if(logs.begin(), logs.end(), std::back_inserter(result),
                 [](const LogEntry& e) { return e.type == LogType::HASH; });
    return result;
}

std::vector<LogEntry> Logger::getBrowserLogs() const {
    std::lock_guard<std::mutex> lock(logMutex);
    std::vector<LogEntry> result;
    std::copy_if(logs.begin(), logs.end(), std::back_inserter(result),
                 [](const LogEntry& e) { return e.type == LogType::BROWSER; });
    return result;
}

std::vector<LogEntry> Logger::getDiscordLogs() const {
    std::lock_guard<std::mutex> lock(logMutex);
    std::vector<LogEntry> result;
    std::copy_if(logs.begin(), logs.end(), std::back_inserter(result),
                 [](const LogEntry& e) { return e.type == LogType::DISCORD; });
    return result;
}

void Logger::clear() {
    std::lock_guard<std::mutex> lock(logMutex);
    logs.clear();
}

void Logger::setLogFile(const std::string& path) {
    std::lock_guard<std::mutex> lock(logMutex);
    logFilePath = path;
}

void Logger::flushToFile() {
    std::lock_guard<std::mutex> lock(logMutex);
    std::ofstream file(logFilePath, std::ios::app);
    if (file.is_open()) {
        for (const auto& entry : logs) {
            file << "[" << entry.timestamp << "] ";
            file << "Type: " << (entry.type == LogType::HASH ? "HASH" : 
                                  entry.type == LogType::BROWSER ? "BROWSER" : "DISCORD");
            file << " | Trigger: " << entry.triggerType;
            file << " | Source: " << entry.source;
            file << " | Value: " << entry.matchedValue;
            file << " | Risk: " << entry.riskWeight << "%\n";
        }
        file.close();
    }
}
