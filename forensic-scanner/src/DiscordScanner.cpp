#include "DiscordScanner.h"
#include "Config.h"
#include "Logger.h"
#include "Utils.h"
#include <fstream>
#include <filesystem>
#include <regex>

namespace fs = std::filesystem;

namespace ForensicScanner {

DiscordScanner::DiscordScanner() {
    m_progress = 0.0f;
    m_matchCount = 0;
    m_stopRequested = false;
}

DiscordScanner::~DiscordScanner() {
    Stop();
}

void DiscordScanner::Scan(std::vector<TriggerEvent>& outTriggers) {
    Logger::Instance().Info("Начало сканирования Discord...", "discord");
    
    m_progress = 0.0f;
    m_matchCount = 0;
    m_stopRequested = false;
    
    // Сканируем локальные данные Discord
    ScanLocalData(outTriggers);
    
    Logger::Instance().Info(
        "Сканирование Discord завершено. Найдено совпадений: " + 
        std::to_string(m_matchCount), "discord");
    
    m_progress = 1.0f;
}

void DiscordScanner::ScanLocalData(std::vector<TriggerEvent>& outTriggers) {
    auto paths = GetDiscordDataPaths();
    
    int totalPaths = static_cast<int>(paths.size());
    int currentPath = 0;
    
    for (const auto& path : paths) {
        if (m_stopRequested) break;
        
        if (!Utils::DirectoryExists(path)) {
            Logger::Instance().Debug("Путь не найден: " + path, "discord");
            continue;
        }
        
        m_currentOperation = "Сканирование: " + Utils::GetFileName(path);
        UpdateProgress(m_progress, m_currentOperation);
        
        // Сканируем Local Storage (LevelDB)
        std::string localStoragePath = path + "\\Local Storage\\leveldb";
        if (Utils::DirectoryExists(localStoragePath)) {
            ScanLevelDB(localStoragePath, outTriggers);
        }
        
        // Сканируем Cache
        std::string cachePath = path + "\\Cache";
        if (Utils::DirectoryExists(cachePath)) {
            ScanCache(cachePath, outTriggers);
        }
        
        // Сканируем логи (если есть)
        std::string logsPath = path + "\\logs";
        if (Utils::DirectoryExists(logsPath)) {
            auto logFiles = Utils::GetFilesInDirectory(logsPath, false);
            for (const auto& logFile : logFiles) {
                if (m_stopRequested) break;
                
                std::string content = Utils::ReadFileContent(logFile);
                if (!content.empty()) {
                    AnalyzeFileContent(content, logFile, outTriggers);
                }
            }
        }
        
        currentPath++;
        m_progress = static_cast<float>(currentPath) / static_cast<float>(totalPaths);
    }
}

void DiscordScanner::ScanExportedLogs(const std::string& filePath,
                                       std::vector<TriggerEvent>& outTriggers) {
    if (!Utils::FileExists(filePath)) {
        Logger::Instance().Warning("Файл не найден: " + filePath, "discord");
        return;
    }
    
    Logger::Instance().Info("Анализ экспортированного лога: " + filePath, "discord");
    
    std::string content = Utils::ReadFileContent(filePath);
    if (content.empty()) {
        return;
    }
    
    AnalyzeFileContent(content, filePath, outTriggers);
}

void DiscordScanner::ScanLevelDB(const std::string& dbPath,
                                  std::vector<TriggerEvent>& outTriggers) {
    // LevelDB хранит данные в .ldb и .log файлах
    // Мы читаем их как текст и ищем паттерны
    
    try {
        for (const auto& entry : fs::directory_iterator(dbPath)) {
            if (m_stopRequested) break;
            
            if (!entry.is_regular_file()) continue;
            
            std::string ext = Utils::ToLower(entry.path().extension().string());
            if (ext != ".ldb" && ext != ".log") continue;
            
            std::string content = Utils::ReadFileContent(entry.path().string());
            if (!content.empty()) {
                AnalyzeFileContent(content, entry.path().string(), outTriggers);
            }
        }
    } catch (const std::exception& e) {
        Logger::Instance().Error(
            "Ошибка при сканировании LevelDB: " + std::string(e.what()), 
            "discord");
    }
}

void DiscordScanner::ScanCache(const std::string& cachePath,
                                std::vector<TriggerEvent>& outTriggers) {
    // Сканируем файлы кеша (ограничиваем размер для производительности)
    try {
        int filesScanned = 0;
        const int maxFiles = 100; // Ограничение для производительности
        
        for (const auto& entry : fs::directory_iterator(cachePath)) {
            if (m_stopRequested || filesScanned >= maxFiles) break;
            
            if (!entry.is_regular_file()) continue;
            
            // Пропускаем большие файлы
            if (entry.file_size() > 1024 * 1024) continue; // > 1 MB
            
            std::string content = Utils::ReadFileContent(entry.path().string());
            if (!content.empty()) {
                AnalyzeFileContent(content, entry.path().string(), outTriggers);
            }
            
            filesScanned++;
        }
    } catch (const std::exception& e) {
        Logger::Instance().Error(
            "Ошибка при сканировании кеша: " + std::string(e.what()), 
            "discord");
    }
}

std::vector<std::string> DiscordScanner::GetDiscordDataPaths() {
    std::vector<std::string> paths;
    
    // Основной Discord
    paths.push_back(Utils::ExpandEnvironmentPath("%APPDATA%\\discord"));
    
    // Discord PTB (Public Test Build)
    paths.push_back(Utils::ExpandEnvironmentPath("%APPDATA%\\discordptb"));
    
    // Discord Canary
    paths.push_back(Utils::ExpandEnvironmentPath("%APPDATA%\\discordcanary"));
    
    // Discord Development
    paths.push_back(Utils::ExpandEnvironmentPath("%APPDATA%\\discorddevelopment"));
    
    return paths;
}

void DiscordScanner::AnalyzeFileContent(const std::string& content,
                                         const std::string& source,
                                         std::vector<TriggerEvent>& outTriggers) {
    // Ищем названия серверов/каналов
    SearchForServerNames(content, source, outTriggers);
    
    // Ищем ключевые слова
    SearchForKeywords(content, source, outTriggers);
}

void DiscordScanner::SearchForServerNames(const std::string& content,
                                           const std::string& source,
                                           std::vector<TriggerEvent>& outTriggers) {
    // Ищем паттерны, похожие на названия серверов Discord
    // Обычно они встречаются в JSON-подобных структурах
    
    // Паттерн для guild/server name
    // Важно: используем raw-string с delimiter, чтобы последовательность )" не ломала литерал.
    std::regex guildPattern(R"re("(?:guild_?name|name|server)"\s*:\s*"([^"]+)")re");
    std::smatch match;
    
    std::string::const_iterator searchStart(content.cbegin());
    while (std::regex_search(searchStart, content.cend(), match, guildPattern)) {
        if (m_stopRequested) break;
        
        std::string serverName = match[1].str();
        
        if (Config::Instance().IsSuspiciousServer(serverName)) {
            TriggerEvent trigger;
            trigger.category = TriggerCategory::Discord;
            trigger.type = TriggerType::SuspiciousServer;
            trigger.source = source;
            trigger.matchedValue = serverName;
            trigger.details = "Подозрительный сервер Discord";
            trigger.weight = Config::Instance().GetWeights().discordServer;
            trigger.timestamp = Utils::GetCurrentTimestamp();
            trigger.context = ExtractContext(content, match.position(), 100);
            
            outTriggers.push_back(trigger);
            m_matchCount++;
            
            Logger::Instance().LogTrigger(trigger);
        }
        
        searchStart = match.suffix().first;
    }
    
    // Паттерн для channel name
    std::regex channelPattern(R"re("(?:channel_?name|channel)"\s*:\s*"([^"]+)")re");
    searchStart = content.cbegin();
    
    while (std::regex_search(searchStart, content.cend(), match, channelPattern)) {
        if (m_stopRequested) break;
        
        std::string channelName = match[1].str();
        
        if (Config::Instance().IsSuspiciousChannel(channelName)) {
            TriggerEvent trigger;
            trigger.category = TriggerCategory::Discord;
            trigger.type = TriggerType::SuspiciousChannel;
            trigger.source = source;
            trigger.matchedValue = channelName;
            trigger.details = "Подозрительный канал Discord";
            trigger.weight = Config::Instance().GetWeights().discordChannel;
            trigger.timestamp = Utils::GetCurrentTimestamp();
            trigger.context = ExtractContext(content, match.position(), 100);
            
            outTriggers.push_back(trigger);
            m_matchCount++;
            
            Logger::Instance().LogTrigger(trigger);
        }
        
        searchStart = match.suffix().first;
    }
}

void DiscordScanner::SearchForKeywords(const std::string& content,
                                        const std::string& source,
                                        std::vector<TriggerEvent>& outTriggers) {
    const auto& keywords = Config::Instance().GetKeywords();
    std::string lowerContent = Utils::ToLower(content);
    
    for (const auto& keyword : keywords) {
        if (m_stopRequested) break;
        
        std::string lowerKeyword = Utils::ToLower(keyword);
        size_t pos = 0;
        int occurrences = 0;
        const int maxOccurrences = 5; // Ограничиваем количество срабатываний на файл
        
        while ((pos = lowerContent.find(lowerKeyword, pos)) != std::string::npos 
               && occurrences < maxOccurrences) {
            
            TriggerEvent trigger;
            trigger.category = TriggerCategory::Discord;
            trigger.type = TriggerType::KeywordFound;
            trigger.source = source;
            trigger.matchedValue = keyword;
            trigger.details = "Ключевое слово в данных Discord";
            trigger.weight = Config::Instance().GetWeights().discordKeyword;
            trigger.timestamp = Utils::GetCurrentTimestamp();
            trigger.context = ExtractContext(content, pos, 100);
            
            outTriggers.push_back(trigger);
            m_matchCount++;
            
            Logger::Instance().LogTrigger(trigger);
            
            pos += lowerKeyword.length();
            occurrences++;
        }
    }
}

std::string DiscordScanner::ExtractContext(const std::string& content, 
                                            size_t matchPos, 
                                            size_t contextLength) {
    size_t start = (matchPos > contextLength / 2) ? matchPos - contextLength / 2 : 0;
    size_t end = std::min(matchPos + contextLength / 2, content.length());
    
    std::string context = content.substr(start, end - start);
    
    // Очищаем от непечатаемых символов
    std::string cleaned;
    for (char c : context) {
        if (std::isprint(static_cast<unsigned char>(c)) || c == ' ') {
            cleaned += c;
        } else {
            cleaned += ' ';
        }
    }
    
    return Utils::Trim(cleaned);
}

void DiscordScanner::SetProgressCallback(
    std::function<void(float, const std::string&)> callback) {
    m_progressCallback = callback;
}

void DiscordScanner::UpdateProgress(float progress, const std::string& operation) {
    if (m_progressCallback) {
        m_progressCallback(progress, operation);
    }
}

} // namespace ForensicScanner
