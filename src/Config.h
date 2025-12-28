#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <vector>
#include <map>

class Config {
public:
    static Config& getInstance();
    
    bool load(const std::string& configPath = "config/config.json");
    
    // Пути для сканирования
    std::vector<std::string> getScanDirectories() const;
    
    // Ключевые слова для поиска
    std::vector<std::string> getKeywords() const;
    
    // Список подозрительных доменов
    std::vector<std::string> getSuspiciousDomains() const;
    
    // Список подозрительных Discord серверов/каналов
    std::vector<std::string> getSuspiciousDiscordServers() const;
    
    // Путь к базе хешей
    std::string getHashDatabasePath() const;
    
    // Веса риска
    int getHashMatchWeight() const { return hashMatchWeight; }
    int getKeywordWeight() const { return keywordWeight; }
    int getDomainWeight() const { return domainWeight; }
    int getDiscordWeight() const { return discordWeight; }
    
    // Пути к файлам браузера/Discord
    std::vector<std::string> getBrowserPaths() const;
    std::vector<std::string> getDiscordPaths() const;

private:
    Config() = default;
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;
    
    std::vector<std::string> scanDirectories;
    std::vector<std::string> keywords;
    std::vector<std::string> suspiciousDomains;
    std::vector<std::string> suspiciousDiscordServers;
    std::string hashDatabasePath;
    
    int hashMatchWeight = 40;
    int keywordWeight = 15;
    int domainWeight = 20;
    int discordWeight = 25;
    
    std::vector<std::string> browserPaths;
    std::vector<std::string> discordPaths;
    
    void loadDefaults();
};

#endif // CONFIG_H
