#include "Config.h"
#include "Utils.h"
#include <fstream>
#include <sstream>
#include <algorithm>

namespace ForensicScanner {

Config& Config::Instance() {
    static Config instance;
    return instance;
}

bool Config::LoadAll(const std::string& configDir) {
    bool success = true;
    
    // Загружаем все конфигурационные файлы
    success &= LoadHashDatabase(configDir + "/hash_database.txt");
    success &= LoadKeywords(configDir + "/keywords.txt");
    success &= LoadBlacklistSites(configDir + "/blacklist_sites.txt");
    success &= LoadDiscordServers(configDir + "/discord_servers.txt");
    success &= LoadScanPaths(configDir + "/scan_paths.txt");
    LoadWeights(configDir + "/weights.txt"); // Опционально
    
    return success;
}

bool Config::LoadHashDatabase(const std::string& path) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::ifstream file(path);
    if (!file.is_open()) {
        // Создаём пустую базу с примерами
        return true;
    }
    
    m_hashDatabase.clear();
    m_hashMap.clear();
    
    std::string line;
    while (std::getline(file, line)) {
        line = Utils::Trim(line);
        if (line.empty() || line[0] == '#') continue;
        
        // Формат: hash|category|name|weight
        auto parts = Utils::Split(line, '|');
        if (parts.size() >= 2) {
            HashEntry entry;
            entry.hash = Utils::ToLower(parts[0]);
            entry.category = parts.size() > 1 ? parts[1] : "unknown";
            entry.name = parts.size() > 2 ? parts[2] : "Unknown";
            entry.weight = parts.size() > 3 ? std::stoi(parts[3]) : m_weights.hashMatch;
            
            m_hashDatabase.push_back(entry);
            m_hashMap[entry.hash] = entry;
        }
    }
    
    return true;
}

bool Config::LoadKeywords(const std::string& path) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto lines = Utils::ReadFileLines(path);
    if (lines.empty()) {
        // Ключевые слова по умолчанию
        m_keywords = {
            "cheat", "cheats", "чит", "читы",
            "hack", "hacks", "хак", "хаки",
            "aimbot", "аимбот",
            "wallhack", "валхак",
            "esp", "еsп",
            "inject", "injector", "инжектор",
            "bypass", "байпас",
            "spoofer", "спуфер",
            "vanish", "godmode", "годмод",
            "mod menu", "модменю",
            "undetected", "ud", "анти детект",
            "hwid", "хвид",
            "rage", "legit", "рейдж", "легит",
            "skin changer", "скин ченджер",
            "triggerbot", "триггербот"
        };
    } else {
        m_keywords = lines;
    }
    
    return true;
}

bool Config::LoadBlacklistSites(const std::string& path) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto lines = Utils::ReadFileLines(path);
    if (lines.empty()) {
        // Примеры сайтов (для демонстрации)
        m_blacklistSites = {
            "unknowncheats.me",
            "mpgh.net",
            "guided-hacking.com",
            "cheatengine.org",
            "wemod.com",
            "gamehacking.org",
            "fearless-assassins.com",
            "cheathappens.com",
            "cheatsquad.gg",
            "projectinfinity.xyz"
        };
    } else {
        m_blacklistSites = lines;
    }
    
    return true;
}

bool Config::LoadDiscordServers(const std::string& path) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::ifstream file(path);
    if (!file.is_open()) {
        // Примеры для демонстрации
        m_discordServers = {
            "cheat", "hack", "bypass", "spoof",
            "inject", "mod menu", "aimbot", "esp"
        };
        m_discordChannels = {
            "cheats", "hacks", "releases", "leaks",
            "showcase", "configs", "support"
        };
        return true;
    }
    
    m_discordServers.clear();
    m_discordChannels.clear();
    
    std::string line;
    bool readingServers = true;
    
    while (std::getline(file, line)) {
        line = Utils::Trim(line);
        if (line.empty() || line[0] == '#') continue;
        
        if (line == "[servers]") {
            readingServers = true;
            continue;
        }
        if (line == "[channels]") {
            readingServers = false;
            continue;
        }
        
        if (readingServers) {
            m_discordServers.push_back(Utils::ToLower(line));
        } else {
            m_discordChannels.push_back(Utils::ToLower(line));
        }
    }
    
    return true;
}

bool Config::LoadWeights(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return false; // Используем значения по умолчанию
    }
    
    std::string line;
    while (std::getline(file, line)) {
        line = Utils::Trim(line);
        if (line.empty() || line[0] == '#') continue;
        
        auto parts = Utils::Split(line, '=');
        if (parts.size() == 2) {
            std::string key = Utils::ToLower(parts[0]);
            int value = std::stoi(parts[1]);
            
            if (key == "hash_match") m_weights.hashMatch = value;
            else if (key == "keyword_found") m_weights.keywordFound = value;
            else if (key == "suspicious_url") m_weights.suspiciousURL = value;
            else if (key == "auth_attempt") m_weights.authAttempt = value;
            else if (key == "discord_server") m_weights.discordServer = value;
            else if (key == "discord_channel") m_weights.discordChannel = value;
            else if (key == "discord_keyword") m_weights.discordKeyword = value;
        }
    }
    
    return true;
}

bool Config::LoadScanPaths(const std::string& path) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto lines = Utils::ReadFileLines(path);
    if (lines.empty()) {
        // Пути по умолчанию
        m_scanPaths = {
            "%APPDATA%\\Microsoft\\Windows\\Recent",
            "C:\\Windows\\Prefetch",
            "%LOCALAPPDATA%\\Temp",
            "%USERPROFILE%\\Downloads"
        };
    } else {
        m_scanPaths = lines;
    }
    
    // Расширяем переменные окружения
    for (auto& scanPath : m_scanPaths) {
        scanPath = Utils::ExpandEnvironmentPath(scanPath);
    }
    
    return true;
}

bool Config::FindHash(const std::string& hash, HashEntry& outEntry) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::string lowerHash = Utils::ToLower(hash);
    auto it = m_hashMap.find(lowerHash);
    if (it != m_hashMap.end()) {
        outEntry = it->second;
        return true;
    }
    return false;
}

bool Config::ContainsKeyword(const std::string& text, std::string& matchedKeyword) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::string lowerText = Utils::ToLower(text);
    
    for (const auto& keyword : m_keywords) {
        if (Utils::Contains(lowerText, keyword, false)) {
            matchedKeyword = keyword;
            return true;
        }
    }
    
    return false;
}

bool Config::IsBlacklistedSite(const std::string& url, std::string& matchedSite) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::string lowerUrl = Utils::ToLower(url);
    
    for (const auto& site : m_blacklistSites) {
        if (Utils::Contains(lowerUrl, site, false)) {
            matchedSite = site;
            return true;
        }
    }
    
    return false;
}

bool Config::IsSuspiciousServer(const std::string& serverName) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::string lowerName = Utils::ToLower(serverName);
    
    for (const auto& server : m_discordServers) {
        if (Utils::Contains(lowerName, server, false)) {
            return true;
        }
    }
    
    return false;
}

bool Config::IsSuspiciousChannel(const std::string& channelName) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::string lowerName = Utils::ToLower(channelName);
    
    for (const auto& channel : m_discordChannels) {
        if (Utils::Contains(lowerName, channel, false)) {
            return true;
        }
    }
    
    return false;
}

std::string Config::GetRecentPath() const {
    return Utils::ExpandEnvironmentPath("%APPDATA%\\Microsoft\\Windows\\Recent");
}

std::string Config::GetPrefetchPath() const {
    return "C:\\Windows\\Prefetch";
}

std::string Config::GetDiscordPath() const {
    return Utils::ExpandEnvironmentPath("%APPDATA%\\discord");
}

std::vector<std::string> Config::GetBrowserPaths() const {
    std::vector<std::string> paths;
    
    // Chrome
    paths.push_back(Utils::ExpandEnvironmentPath(
        "%LOCALAPPDATA%\\Google\\Chrome\\User Data\\Default"));
    
    // Firefox
    paths.push_back(Utils::ExpandEnvironmentPath(
        "%APPDATA%\\Mozilla\\Firefox\\Profiles"));
    
    // Edge
    paths.push_back(Utils::ExpandEnvironmentPath(
        "%LOCALAPPDATA%\\Microsoft\\Edge\\User Data\\Default"));
    
    // Opera
    paths.push_back(Utils::ExpandEnvironmentPath(
        "%APPDATA%\\Opera Software\\Opera Stable"));
    
    // Brave
    paths.push_back(Utils::ExpandEnvironmentPath(
        "%LOCALAPPDATA%\\BraveSoftware\\Brave-Browser\\User Data\\Default"));
    
    return paths;
}

} // namespace ForensicScanner
