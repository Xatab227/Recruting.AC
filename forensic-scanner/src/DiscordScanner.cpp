/**
 * @file DiscordScanner.cpp
 * @brief Реализация модуля анализа Discord
 */

#include "../include/DiscordScanner.h"
#include "../include/Logger.h"
#include "../include/Config.h"
#include <Windows.h>
#include <fstream>
#include <sstream>
#include <algorithm>

namespace ForensicScanner {

DiscordScanner::DiscordScanner() : m_riskWeight(25) {
    m_discordPath = getDefaultDiscordPath();
}

DiscordScanner::~DiscordScanner() {
}

void DiscordScanner::setKeywords(const std::vector<std::wstring>& keywords) {
    m_keywords = keywords;
}

void DiscordScanner::setBlacklist(const std::vector<std::wstring>& blacklist) {
    m_blacklist = blacklist;
}

void DiscordScanner::setDiscordPath(const std::wstring& path) {
    m_discordPath = Config::expandEnvironmentPath(path);
}

void DiscordScanner::setRiskWeight(int weight) {
    m_riskWeight = weight;
}

std::wstring DiscordScanner::getDefaultDiscordPath() {
    return Config::expandEnvironmentPath(L"%APPDATA%\\discord");
}

std::vector<DiscordMatch> DiscordScanner::scan(DiscordScanCallback callback) {
    m_matches.clear();
    
    if (callback) callback(L"Сканирование LevelDB...", 0);
    std::vector<DiscordMatch> levelDbMatches = scanLevelDB();
    m_matches.insert(m_matches.end(), levelDbMatches.begin(), levelDbMatches.end());
    
    if (callback) callback(L"Сканирование кеша...", 33);
    std::vector<DiscordMatch> cacheMatches = scanCache();
    m_matches.insert(m_matches.end(), cacheMatches.begin(), cacheMatches.end());
    
    if (callback) callback(L"Сканирование LocalStorage...", 66);
    std::vector<DiscordMatch> storageMatches = scanLocalStorage();
    m_matches.insert(m_matches.end(), storageMatches.begin(), storageMatches.end());
    
    if (callback) callback(L"Завершено", 100);
    
    // Логируем все найденные совпадения
    for (const auto& match : m_matches) {
        LogEntry entry;
        entry.category = LogCategory::DISCORD;
        
        switch (match.matchType) {
            case DiscordMatchType::SERVER_NAME:
                entry.triggerType = TriggerType::DISCORD_SERVER;
                break;
            case DiscordMatchType::CHANNEL_NAME:
                entry.triggerType = TriggerType::DISCORD_CHANNEL;
                break;
            default:
                entry.triggerType = TriggerType::DISCORD_KEYWORD;
                break;
        }
        
        entry.source = match.source;
        entry.matchedValue = match.matchedValue;
        entry.riskWeight = match.riskWeight;
        
        std::wstring details = L"Тип: " + matchTypeToString(match.matchType);
        if (!match.serverName.empty()) {
            details += L", Сервер: " + match.serverName;
        }
        if (!match.channelName.empty()) {
            details += L", Канал: " + match.channelName;
        }
        if (!match.context.empty()) {
            details += L", Контекст: " + match.context;
        }
        entry.details = details;
        
        Logger::getInstance().addEntry(entry);
    }
    
    return m_matches;
}

std::vector<DiscordMatch> DiscordScanner::scanLevelDB() {
    std::vector<DiscordMatch> results;
    
    std::wstring levelDbPath = m_discordPath + L"\\Local Storage\\leveldb";
    std::vector<std::wstring> files = getFilesRecursive(levelDbPath);
    
    for (const auto& file : files) {
        // Сканируем .ldb и .log файлы
        if (file.find(L".ldb") != std::wstring::npos || 
            file.find(L".log") != std::wstring::npos) {
            std::vector<DiscordMatch> fileMatches = scanBinaryFile(file);
            for (auto& match : fileMatches) {
                match.matchType = DiscordMatchType::LEVELDB_ENTRY;
            }
            results.insert(results.end(), fileMatches.begin(), fileMatches.end());
        }
    }
    
    return results;
}

std::vector<DiscordMatch> DiscordScanner::scanCache() {
    std::vector<DiscordMatch> results;
    
    std::wstring cachePath = m_discordPath + L"\\Cache";
    std::vector<std::wstring> files = getFilesRecursive(cachePath);
    
    for (const auto& file : files) {
        std::vector<DiscordMatch> fileMatches = scanBinaryFile(file);
        for (auto& match : fileMatches) {
            match.matchType = DiscordMatchType::CACHE_FILE;
        }
        results.insert(results.end(), fileMatches.begin(), fileMatches.end());
    }
    
    // Также проверяем Cache2
    std::wstring cache2Path = m_discordPath + L"\\Code Cache";
    files = getFilesRecursive(cache2Path);
    
    for (const auto& file : files) {
        std::vector<DiscordMatch> fileMatches = scanBinaryFile(file);
        for (auto& match : fileMatches) {
            match.matchType = DiscordMatchType::CACHE_FILE;
        }
        results.insert(results.end(), fileMatches.begin(), fileMatches.end());
    }
    
    return results;
}

std::vector<DiscordMatch> DiscordScanner::scanLocalStorage() {
    std::vector<DiscordMatch> results;
    
    std::wstring storagePath = m_discordPath + L"\\Local Storage";
    std::vector<std::wstring> files = getFilesRecursive(storagePath);
    
    for (const auto& file : files) {
        std::vector<DiscordMatch> fileMatches = scanBinaryFile(file);
        results.insert(results.end(), fileMatches.begin(), fileMatches.end());
    }
    
    return results;
}

std::vector<DiscordMatch> DiscordScanner::scanExportedLogs(const std::wstring& logPath) {
    return scanTextFile(logPath);
}

std::vector<DiscordMatch> DiscordScanner::scanTextFile(const std::wstring& filePath) {
    std::vector<DiscordMatch> results;
    
    std::wifstream file(filePath);
    if (!file.is_open()) {
        return results;
    }
    
    std::wstring line;
    int lineNum = 0;
    
    while (std::getline(file, line)) {
        lineNum++;
        
        std::wstring matchedValue;
        DiscordMatchType matchType;
        
        if (checkText(line, matchedValue, matchType)) {
            DiscordMatch match;
            match.matchType = matchType;
            match.source = filePath;
            match.matchedValue = matchedValue;
            match.context = line.substr(0, 100); // Первые 100 символов
            match.riskWeight = m_riskWeight;
            
            results.push_back(match);
        }
    }
    
    file.close();
    return results;
}

std::vector<DiscordMatch> DiscordScanner::scanBinaryFile(const std::wstring& filePath) {
    std::vector<DiscordMatch> results;
    
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        return results;
    }
    
    // Читаем файл блоками
    const size_t bufferSize = 65536;
    std::vector<char> buffer(bufferSize);
    
    while (file.read(buffer.data(), bufferSize) || file.gcount() > 0) {
        size_t bytesRead = static_cast<size_t>(file.gcount());
        
        // Преобразуем в wstring для анализа
        std::wstring content(buffer.begin(), buffer.begin() + bytesRead);
        std::wstring lowerContent = toLower(content);
        
        // Проверяем ключевые слова
        for (const auto& keyword : m_keywords) {
            std::wstring lowerKeyword = toLower(keyword);
            size_t pos = lowerContent.find(lowerKeyword);
            
            if (pos != std::wstring::npos) {
                DiscordMatch match;
                match.matchType = DiscordMatchType::MESSAGE_KEYWORD;
                match.source = filePath;
                match.matchedValue = keyword;
                match.context = extractContext(content, pos);
                match.riskWeight = m_riskWeight;
                
                results.push_back(match);
            }
        }
        
        // Проверяем чёрный список серверов/каналов
        for (const auto& item : m_blacklist) {
            std::wstring lowerItem = toLower(item);
            size_t pos = lowerContent.find(lowerItem);
            
            if (pos != std::wstring::npos) {
                DiscordMatch match;
                match.matchType = DiscordMatchType::SERVER_NAME;
                match.source = filePath;
                match.matchedValue = item;
                match.serverName = item;
                match.context = extractContext(content, pos);
                match.riskWeight = m_riskWeight + 5; // Немного выше для серверов
                
                results.push_back(match);
            }
        }
    }
    
    file.close();
    return results;
}

bool DiscordScanner::checkText(const std::wstring& text, std::wstring& outMatch, 
                                DiscordMatchType& outType) {
    std::wstring lowerText = toLower(text);
    
    // Сначала проверяем чёрный список серверов/каналов
    for (const auto& item : m_blacklist) {
        std::wstring lowerItem = toLower(item);
        if (lowerText.find(lowerItem) != std::wstring::npos) {
            outMatch = item;
            outType = DiscordMatchType::SERVER_NAME;
            return true;
        }
    }
    
    // Затем проверяем ключевые слова
    for (const auto& keyword : m_keywords) {
        std::wstring lowerKeyword = toLower(keyword);
        if (lowerText.find(lowerKeyword) != std::wstring::npos) {
            outMatch = keyword;
            outType = DiscordMatchType::MESSAGE_KEYWORD;
            return true;
        }
    }
    
    return false;
}

std::wstring DiscordScanner::toLower(const std::wstring& str) {
    std::wstring result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::towlower);
    return result;
}

std::vector<std::wstring> DiscordScanner::getFilesRecursive(const std::wstring& directory) {
    std::vector<std::wstring> files;
    
    std::wstring searchPath = directory;
    if (!searchPath.empty() && searchPath.back() != L'\\') {
        searchPath += L'\\';
    }
    searchPath += L'*';
    
    WIN32_FIND_DATAW findData;
    HANDLE hFind = FindFirstFileW(searchPath.c_str(), &findData);
    
    if (hFind == INVALID_HANDLE_VALUE) {
        return files;
    }
    
    do {
        std::wstring name = findData.cFileName;
        
        if (name == L"." || name == L"..") {
            continue;
        }
        
        std::wstring fullPath = directory;
        if (!fullPath.empty() && fullPath.back() != L'\\') {
            fullPath += L'\\';
        }
        fullPath += name;
        
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            // Рекурсивно обходим поддиректории
            std::vector<std::wstring> subFiles = getFilesRecursive(fullPath);
            files.insert(files.end(), subFiles.begin(), subFiles.end());
        } else {
            files.push_back(fullPath);
        }
    } while (FindNextFileW(hFind, &findData));
    
    FindClose(hFind);
    return files;
}

std::wstring DiscordScanner::extractContext(const std::wstring& text, size_t pos, size_t contextSize) {
    size_t start = (pos > contextSize) ? pos - contextSize : 0;
    size_t end = std::min(pos + contextSize, text.length());
    
    std::wstring context = text.substr(start, end - start);
    
    // Очищаем от непечатаемых символов
    for (wchar_t& c : context) {
        if (c < 32 || c > 126) {
            c = L' ';
        }
    }
    
    return context;
}

const std::vector<DiscordMatch>& DiscordScanner::getMatches() const {
    return m_matches;
}

size_t DiscordScanner::getMatchCount() const {
    return m_matches.size();
}

void DiscordScanner::clearResults() {
    m_matches.clear();
}

std::wstring DiscordScanner::matchTypeToString(DiscordMatchType type) {
    switch (type) {
        case DiscordMatchType::SERVER_NAME:     return L"Название сервера";
        case DiscordMatchType::CHANNEL_NAME:    return L"Название канала";
        case DiscordMatchType::MESSAGE_KEYWORD: return L"Ключевое слово";
        case DiscordMatchType::CACHE_FILE:      return L"Файл кеша";
        case DiscordMatchType::LEVELDB_ENTRY:   return L"Запись LevelDB";
        default:                                return L"Неизвестно";
    }
}

} // namespace ForensicScanner
