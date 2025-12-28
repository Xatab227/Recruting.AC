/**
 * @file Config.cpp
 * @brief Реализация модуля конфигурации
 */

#include "../include/Config.h"
#include <fstream>
#include <sstream>
#include <algorithm>

namespace ForensicScanner {

Config::Config() {
    setDefaults();
}

Config::~Config() {
}

void Config::setDefaults() {
    // Пути для сканирования по умолчанию
    m_data.scanDirectories.clear();
    m_data.scanDirectories.push_back(L"%APPDATA%\\Microsoft\\Windows\\Recent");
    m_data.scanDirectories.push_back(L"C:\\Windows\\Prefetch");
    
    // Путь к базе хешей
    m_data.hashDatabasePath = L"config\\hash_database.txt";
    
    // Путь к файлу логов
    m_data.logFilePath = L"scan_log.txt";
    
    // Ключевые слова по умолчанию
    m_data.keywords.clear();
    m_data.keywords.push_back(L"чит");
    m_data.keywords.push_back(L"cheat");
    m_data.keywords.push_back(L"vanish");
    m_data.keywords.push_back(L"hack");
    m_data.keywords.push_back(L"inject");
    m_data.keywords.push_back(L"aimbot");
    m_data.keywords.push_back(L"wallhack");
    m_data.keywords.push_back(L"esp");
    m_data.keywords.push_back(L"bypass");
    m_data.keywords.push_back(L"spoofer");
    
    // Веса по умолчанию
    m_data.hashMatchWeight = 40;
    m_data.keywordMatchWeight = 15;
    m_data.siteVisitWeight = 20;
    m_data.discordMatchWeight = 25;
    
    // Пути к браузерам (будут расширены)
    m_data.browserPaths.clear();
    m_data.browserPaths.push_back(L"%LOCALAPPDATA%\\Google\\Chrome\\User Data\\Default\\History");
    m_data.browserPaths.push_back(L"%LOCALAPPDATA%\\Microsoft\\Edge\\User Data\\Default\\History");
    m_data.browserPaths.push_back(L"%APPDATA%\\Mozilla\\Firefox\\Profiles");
    m_data.browserPaths.push_back(L"%APPDATA%\\Opera Software\\Opera Stable\\History");
    
    // Путь к Discord
    m_data.discordPath = L"%APPDATA%\\discord";
    
    // Чёрный список сайтов по умолчанию
    m_data.blacklistSites.clear();
    m_data.blacklistSites.push_back(L"unknowncheats.me");
    m_data.blacklistSites.push_back(L"mpgh.net");
    m_data.blacklistSites.push_back(L"cheatengine.org");
    m_data.blacklistSites.push_back(L"gamehacking.com");
    m_data.blacklistSites.push_back(L"wemod.com");
    m_data.blacklistSites.push_back(L"cheatautomation.com");
    m_data.blacklistSites.push_back(L"aimjunkies.com");
    m_data.blacklistSites.push_back(L"artificialaiming.net");
    m_data.blacklistSites.push_back(L"projectbase.cc");
    m_data.blacklistSites.push_back(L"elitepvpers.com");
    
    // Чёрный список Discord серверов/каналов
    m_data.discordBlacklist.clear();
    m_data.discordBlacklist.push_back(L"cheat");
    m_data.discordBlacklist.push_back(L"hack");
    m_data.discordBlacklist.push_back(L"bypass");
    m_data.discordBlacklist.push_back(L"spoof");
    m_data.discordBlacklist.push_back(L"inject");
}

bool Config::loadConfig(const std::wstring& configDir) {
    // Загружаем ключевые слова
    std::wstring keywordsPath = configDir + L"\\keywords.txt";
    loadKeywords(keywordsPath);
    
    // Загружаем чёрный список сайтов
    std::wstring sitesPath = configDir + L"\\blacklist_sites.txt";
    loadBlacklistSites(sitesPath);
    
    // Загружаем чёрный список Discord
    std::wstring discordPath = configDir + L"\\discord_blacklist.txt";
    loadDiscordBlacklist(discordPath);
    
    // Обновляем путь к базе хешей
    m_data.hashDatabasePath = configDir + L"\\hash_database.txt";
    
    return true;
}

bool Config::loadKeywords(const std::wstring& filePath) {
    std::vector<std::wstring> loaded;
    if (loadStringList(filePath, loaded) && !loaded.empty()) {
        m_data.keywords = loaded;
        return true;
    }
    return false;
}

bool Config::loadBlacklistSites(const std::wstring& filePath) {
    std::vector<std::wstring> loaded;
    if (loadStringList(filePath, loaded) && !loaded.empty()) {
        m_data.blacklistSites = loaded;
        return true;
    }
    return false;
}

bool Config::loadDiscordBlacklist(const std::wstring& filePath) {
    std::vector<std::wstring> loaded;
    if (loadStringList(filePath, loaded) && !loaded.empty()) {
        m_data.discordBlacklist = loaded;
        return true;
    }
    return false;
}

bool Config::loadStringList(const std::wstring& filePath, std::vector<std::wstring>& outList) {
    std::wifstream file(filePath);
    if (!file.is_open()) {
        return false;
    }
    
    outList.clear();
    std::wstring line;
    while (std::getline(file, line)) {
        // Пропускаем пустые строки и комментарии
        if (line.empty() || line[0] == L'#') {
            continue;
        }
        
        // Удаляем пробелы в начале и конце
        size_t start = line.find_first_not_of(L" \t\r\n");
        size_t end = line.find_last_not_of(L" \t\r\n");
        
        if (start != std::wstring::npos && end != std::wstring::npos) {
            outList.push_back(line.substr(start, end - start + 1));
        }
    }
    
    file.close();
    return true;
}

const ConfigData& Config::getData() const {
    return m_data;
}

std::wstring Config::expandEnvironmentPath(const std::wstring& path) {
    wchar_t expandedPath[MAX_PATH];
    DWORD result = ExpandEnvironmentStringsW(path.c_str(), expandedPath, MAX_PATH);
    
    if (result > 0 && result < MAX_PATH) {
        return std::wstring(expandedPath);
    }
    
    return path;
}

} // namespace ForensicScanner
