#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <vector>
#include <fstream>
#include <mutex>
#include <ctime>

enum class LogType {
    HASH,
    BROWSER,
    DISCORD
};

struct LogEntry {
    LogType type;
    std::string triggerType;
    std::string source;
    std::string matchedValue;
    int riskWeight;
    std::string timestamp;
    
    LogEntry(LogType t, const std::string& tt, const std::string& src, 
             const std::string& val, int weight)
        : type(t), triggerType(tt), source(src), matchedValue(val), riskWeight(weight) {
        time_t now = time(0);
        char* dt = ctime(&now);
        timestamp = std::string(dt);
        timestamp.pop_back(); // Удалить \n
    }
};

class Logger {
public:
    static Logger& getInstance();
    
    void log(LogType type, const std::string& triggerType, 
             const std::string& source, const std::string& matchedValue, int riskWeight);
    
    std::vector<LogEntry> getHashLogs() const;
    std::vector<LogEntry> getBrowserLogs() const;
    std::vector<LogEntry> getDiscordLogs() const;
    
    void clear();
    
    void setLogFile(const std::string& path);
    void flushToFile();

private:
    Logger() = default;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    std::vector<LogEntry> logs;
    std::string logFilePath = "scan_log.txt";
    mutable std::mutex logMutex;
};

#endif // LOGGER_H
