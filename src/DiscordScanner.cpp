#include "DiscordScanner.h"
#include "Logger.h"
#include "Config.h"
#include <fstream>
#include <filesystem>
#include <algorithm>

DiscordScanner::DiscordScanner() {
}

std::string DiscordScanner::toLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

void DiscordScanner::checkKeywords(const std::string& content, const std::string& source) {
    Config& config = Config::getInstance();
    std::string lowerContent = toLower(content);
    
    for (const auto& keyword : config.getKeywords()) {
        std::string lowerKeyword = toLower(keyword);
        if (lowerContent.find(lowerKeyword) != std::string::npos) {
            matchCount++;
            Logger::getInstance().log(
                LogType::DISCORD,
                "Keyword Match",
                source,
                keyword,
                config.getDiscordWeight()
            );
        }
    }
}

void DiscordScanner::checkServers(const std::string& content, const std::string& source) {
    Config& config = Config::getInstance();
    std::string lowerContent = toLower(content);
    
    for (const auto& server : config.getSuspiciousDiscordServers()) {
        std::string lowerServer = toLower(server);
        if (lowerContent.find(lowerServer) != std::string::npos) {
            matchCount++;
            Logger::getInstance().log(
                LogType::DISCORD,
                "Server Match",
                source,
                server,
                config.getDiscordWeight()
            );
        }
    }
}

void DiscordScanner::scanDiscordFiles(const std::string& discordPath, std::function<void(int, int)> progressCallback) {
    if (!std::filesystem::exists(discordPath)) {
        return;
    }
    
    int fileCount = 0;
    int processedCount = 0;
    
    // Подсчет файлов
    for (const auto& entry : std::filesystem::recursive_directory_iterator(discordPath)) {
        if (entry.is_regular_file()) {
            fileCount++;
        }
    }
    
    // Сканирование файлов
    for (const auto& entry : std::filesystem::recursive_directory_iterator(discordPath)) {
        if (entry.is_regular_file()) {
            try {
                std::ifstream file(entry.path(), std::ios::binary);
                if (file.is_open()) {
                    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
                    file.close();
                    
                    checkKeywords(content, entry.path().string());
                    checkServers(content, entry.path().string());
                }
            } catch (...) {
                // Игнорируем ошибки
            }
            
            processedCount++;
            if (progressCallback && processedCount % 10 == 0) {
                progressCallback(processedCount, fileCount);
            }
        }
    }
}

void DiscordScanner::scanDiscordLogs(const std::string& logPath, std::function<void(int, int)> progressCallback) {
    if (!std::filesystem::exists(logPath)) {
        return;
    }
    
    std::ifstream file(logPath);
    if (!file.is_open()) {
        return;
    }
    
    std::string line;
    int lineCount = 0;
    while (std::getline(file, line)) {
        lineCount++;
        checkKeywords(line, logPath);
        checkServers(line, logPath);
        
        if (progressCallback && lineCount % 100 == 0) {
            progressCallback(lineCount, lineCount);
        }
    }
    file.close();
}
