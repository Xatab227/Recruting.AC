#include "Config.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <windows.h>

#ifdef _WIN32
#include <shlobj.h>
#endif

Config& Config::getInstance() {
    static Config instance;
    return instance;
}

bool Config::load(const std::string& configPath) {
    loadDefaults();
    
    // Попытка загрузить из файла, если он существует
    std::ifstream file(configPath);
    if (file.is_open()) {
        // Простой парсинг JSON (упрощенный)
        std::string line;
        while (std::getline(file, line)) {
            // Здесь можно добавить более сложный парсинг JSON
            // Для простоты используем дефолтные значения
        }
        file.close();
    }
    
    return true;
}

void Config::loadDefaults() {
    // Стандартные директории для сканирования
    scanDirectories.clear();
    
    // %APPDATA%\Microsoft\Windows\Recent
    char appDataPath[MAX_PATH];
    if (SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, appDataPath) == S_OK) {
        std::string recentPath = std::string(appDataPath) + "\\Microsoft\\Windows\\Recent";
        scanDirectories.push_back(recentPath);
    }
    
    // C:\Windows\Prefetch
    scanDirectories.push_back("C:\\Windows\\Prefetch");
    
    // Ключевые слова
    keywords = {"чит", "cheat", "vanish", "hack", "mod", "injector", "bypass", "aimbot"};
    
    // Подозрительные домены
    suspiciousDomains = {
        "cheatengine.org",
        "unknowncheats.me",
        "guidedhacking.com",
        "mpgh.net"
    };
    
    // Подозрительные Discord серверы
    suspiciousDiscordServers = {
        "cheat",
        "hack",
        "mod",
        "bypass"
    };
    
    // Путь к базе хешей
    hashDatabasePath = "config/hash_database.txt";
    
    // Пути к файлам браузера
    browserPaths.clear();
    if (SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, appDataPath) == S_OK) {
        browserPaths.push_back(std::string(appDataPath) + "\\Google\\Chrome\\User Data\\Default\\History");
        browserPaths.push_back(std::string(appDataPath) + "\\Microsoft\\Edge\\User Data\\Default\\History");
    }
    
    // Пути к файлам Discord
    discordPaths.clear();
    if (SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, appDataPath) == S_OK) {
        discordPaths.push_back(std::string(appDataPath) + "\\discord\\Local Storage\\leveldb");
        discordPaths.push_back(std::string(appDataPath) + "\\discord\\Cache");
    }
}

std::vector<std::string> Config::getScanDirectories() const {
    return scanDirectories;
}

std::vector<std::string> Config::getKeywords() const {
    return keywords;
}

std::vector<std::string> Config::getSuspiciousDomains() const {
    return suspiciousDomains;
}

std::vector<std::string> Config::getSuspiciousDiscordServers() const {
    return suspiciousDiscordServers;
}

std::string Config::getHashDatabasePath() const {
    return hashDatabasePath;
}

std::vector<std::string> Config::getBrowserPaths() const {
    return browserPaths;
}

std::vector<std::string> Config::getDiscordPaths() const {
    return discordPaths;
}
