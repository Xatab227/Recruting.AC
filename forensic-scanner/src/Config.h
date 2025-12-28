#pragma once

#include <string>
#include <vector>
#include <map>
#include <mutex>

namespace ForensicScanner {

// Запись в базе хешей
struct HashEntry {
    std::string hash;
    std::string category;   // cheats, modmenu, injectors и т.п.
    std::string name;       // Название программы/файла
    int weight;             // Вес (влияние на риск)
};

// Конфигурация весов триггеров
struct TriggerWeights {
    int hashMatch = 40;             // Совпадение хеша
    int keywordFound = 15;          // Найдено ключевое слово
    int suspiciousURL = 20;         // Подозрительный URL
    int authAttempt = 25;           // Попытка авторизации
    int discordServer = 30;         // Подозрительный сервер Discord
    int discordChannel = 20;        // Подозрительный канал Discord
    int discordKeyword = 15;        // Ключевое слово в Discord
};

class Config {
public:
    static Config& Instance();
    
    // Загрузка конфигурации
    bool LoadAll(const std::string& configDir = "config");
    bool LoadHashDatabase(const std::string& path);
    bool LoadKeywords(const std::string& path);
    bool LoadBlacklistSites(const std::string& path);
    bool LoadDiscordServers(const std::string& path);
    bool LoadWeights(const std::string& path);
    bool LoadScanPaths(const std::string& path);
    
    // Геттеры
    const std::vector<HashEntry>& GetHashDatabase() const { return m_hashDatabase; }
    const std::vector<std::string>& GetKeywords() const { return m_keywords; }
    const std::vector<std::string>& GetBlacklistSites() const { return m_blacklistSites; }
    const std::vector<std::string>& GetDiscordServers() const { return m_discordServers; }
    const std::vector<std::string>& GetDiscordChannels() const { return m_discordChannels; }
    const std::vector<std::string>& GetScanPaths() const { return m_scanPaths; }
    const TriggerWeights& GetWeights() const { return m_weights; }
    
    // Поиск хеша в базе
    bool FindHash(const std::string& hash, HashEntry& outEntry) const;
    
    // Проверка наличия ключевого слова в строке
    bool ContainsKeyword(const std::string& text, std::string& matchedKeyword) const;
    
    // Проверка URL на вхождение в чёрный список
    bool IsBlacklistedSite(const std::string& url, std::string& matchedSite) const;
    
    // Проверка Discord сервера/канала
    bool IsSuspiciousServer(const std::string& serverName) const;
    bool IsSuspiciousChannel(const std::string& channelName) const;
    
    // Пути по умолчанию
    std::string GetRecentPath() const;
    std::string GetPrefetchPath() const;
    std::string GetDiscordPath() const;
    std::vector<std::string> GetBrowserPaths() const;

private:
    Config() = default;
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;
    
    std::vector<HashEntry> m_hashDatabase;
    std::vector<std::string> m_keywords;
    std::vector<std::string> m_blacklistSites;
    std::vector<std::string> m_discordServers;
    std::vector<std::string> m_discordChannels;
    std::vector<std::string> m_scanPaths;
    TriggerWeights m_weights;
    
    std::map<std::string, HashEntry> m_hashMap; // Для быстрого поиска
    
    mutable std::mutex m_mutex;
};

} // namespace ForensicScanner
