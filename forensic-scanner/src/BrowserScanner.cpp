/**
 * @file BrowserScanner.cpp
 * @brief Реализация модуля анализа браузера
 */

#include "../include/BrowserScanner.h"
#include "../include/Logger.h"
#include "../include/Config.h"
#include <Windows.h>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <regex>

namespace ForensicScanner {

BrowserScanner::BrowserScanner() 
    : m_keywordWeight(15), m_siteVisitWeight(20) {
}

BrowserScanner::~BrowserScanner() {
}

void BrowserScanner::setKeywords(const std::vector<std::wstring>& keywords) {
    m_keywords = keywords;
}

void BrowserScanner::setBlacklistSites(const std::vector<std::wstring>& sites) {
    m_blacklistSites = sites;
}

void BrowserScanner::setKeywordWeight(int weight) {
    m_keywordWeight = weight;
}

void BrowserScanner::setSiteVisitWeight(int weight) {
    m_siteVisitWeight = weight;
}

std::vector<BrowserMatch> BrowserScanner::scanAllBrowsers(BrowserScanCallback callback) {
    m_matches.clear();
    
    if (callback) callback(L"Сканирование Chrome...", 0);
    std::vector<BrowserMatch> chromeMatches = scanChrome();
    m_matches.insert(m_matches.end(), chromeMatches.begin(), chromeMatches.end());
    
    if (callback) callback(L"Сканирование Edge...", 25);
    std::vector<BrowserMatch> edgeMatches = scanEdge();
    m_matches.insert(m_matches.end(), edgeMatches.begin(), edgeMatches.end());
    
    if (callback) callback(L"Сканирование Firefox...", 50);
    std::vector<BrowserMatch> firefoxMatches = scanFirefox();
    m_matches.insert(m_matches.end(), firefoxMatches.begin(), firefoxMatches.end());
    
    if (callback) callback(L"Сканирование Opera...", 75);
    std::vector<BrowserMatch> operaMatches = scanOpera();
    m_matches.insert(m_matches.end(), operaMatches.begin(), operaMatches.end());
    
    if (callback) callback(L"Завершено", 100);
    
    // Логируем все найденные совпадения
    for (const auto& match : m_matches) {
        LogEntry entry;
        entry.category = LogCategory::BROWSER;
        entry.triggerType = (match.matchType == L"keyword") ? 
                           TriggerType::KEYWORD_MATCH : TriggerType::URL_MATCH;
        entry.source = match.source;
        entry.matchedValue = match.matchedValue;
        entry.details = L"Контекст: " + match.context;
        entry.riskWeight = match.riskWeight;
        
        if (match.isAuthAttempt) {
            entry.details += L" [ПОПЫТКА АВТОРИЗАЦИИ]";
            entry.riskWeight += 10; // Увеличиваем риск для авторизации
        }
        
        Logger::getInstance().addEntry(entry);
    }
    
    return m_matches;
}

std::vector<BrowserMatch> BrowserScanner::scanChrome() {
    std::wstring profilePath = Config::expandEnvironmentPath(
        L"%LOCALAPPDATA%\\Google\\Chrome\\User Data\\Default");
    return scanChromiumBrowser(profilePath, BrowserType::CHROME);
}

std::vector<BrowserMatch> BrowserScanner::scanEdge() {
    std::wstring profilePath = Config::expandEnvironmentPath(
        L"%LOCALAPPDATA%\\Microsoft\\Edge\\User Data\\Default");
    return scanChromiumBrowser(profilePath, BrowserType::EDGE);
}

std::vector<BrowserMatch> BrowserScanner::scanOpera() {
    std::wstring profilePath = Config::expandEnvironmentPath(
        L"%APPDATA%\\Opera Software\\Opera Stable");
    return scanChromiumBrowser(profilePath, BrowserType::OPERA);
}

std::vector<BrowserMatch> BrowserScanner::scanFirefox() {
    std::vector<BrowserMatch> results;
    
    std::wstring profilesPath = Config::expandEnvironmentPath(
        L"%APPDATA%\\Mozilla\\Firefox\\Profiles");
    
    // Ищем все профили Firefox
    WIN32_FIND_DATAW findData;
    std::wstring searchPath = profilesPath + L"\\*";
    HANDLE hFind = FindFirstFileW(searchPath.c_str(), &findData);
    
    if (hFind == INVALID_HANDLE_VALUE) {
        return results;
    }
    
    do {
        std::wstring dirName = findData.cFileName;
        
        if (dirName == L"." || dirName == L"..") {
            continue;
        }
        
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            // Проверяем places.sqlite в каждом профиле
            std::wstring placesPath = profilesPath + L"\\" + dirName + L"\\places.sqlite";
            
            // Firefox использует SQLite, поэтому сканируем как текст
            std::vector<BrowserMatch> profileMatches = scanTextFile(placesPath, BrowserType::FIREFOX);
            results.insert(results.end(), profileMatches.begin(), profileMatches.end());
        }
    } while (FindNextFileW(hFind, &findData));
    
    FindClose(hFind);
    
    return results;
}

std::vector<BrowserMatch> BrowserScanner::scanChromiumBrowser(const std::wstring& profilePath, 
                                                              BrowserType browserType) {
    std::vector<BrowserMatch> results;
    
    // Сканируем History
    std::wstring historyPath = profilePath + L"\\History";
    std::vector<BrowserMatch> historyMatches = scanTextFile(historyPath, browserType);
    results.insert(results.end(), historyMatches.begin(), historyMatches.end());
    
    // Сканируем Visited Links
    std::wstring visitedPath = profilePath + L"\\Visited Links";
    std::vector<BrowserMatch> visitedMatches = scanTextFile(visitedPath, browserType);
    results.insert(results.end(), visitedMatches.begin(), visitedMatches.end());
    
    // Сканируем Top Sites
    std::wstring topSitesPath = profilePath + L"\\Top Sites";
    std::vector<BrowserMatch> topSitesMatches = scanTextFile(topSitesPath, browserType);
    results.insert(results.end(), topSitesMatches.begin(), topSitesMatches.end());
    
    return results;
}

std::vector<BrowserMatch> BrowserScanner::scanMyActivity(const std::wstring& filePath) {
    return scanTextFile(filePath, BrowserType::MY_ACTIVITY);
}

std::vector<BrowserMatch> BrowserScanner::scanTextFile(const std::wstring& filePath, 
                                                        BrowserType browserType) {
    std::vector<BrowserMatch> results;
    
    // Открываем файл как бинарный для поиска строк
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        return results;
    }
    
    // Читаем весь файл
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    file.close();
    
    // Конвертируем в wstring для анализа
    std::wstring wContent(content.begin(), content.end());
    std::wstring lowerContent = toLower(wContent);
    
    // Ищем ключевые слова
    for (const auto& keyword : m_keywords) {
        std::wstring lowerKeyword = toLower(keyword);
        size_t pos = 0;
        
        while ((pos = lowerContent.find(lowerKeyword, pos)) != std::wstring::npos) {
            BrowserMatch match;
            match.browserType = browserType;
            match.source = filePath;
            match.matchType = L"keyword";
            match.matchedValue = keyword;
            match.riskWeight = m_keywordWeight;
            
            // Извлекаем контекст
            size_t start = (pos > 30) ? pos - 30 : 0;
            size_t len = 60 + keyword.length();
            match.context = wContent.substr(start, len);
            
            // Очищаем контекст от непечатаемых символов
            for (wchar_t& c : match.context) {
                if (c < 32 || c > 126) c = L' ';
            }
            
            results.push_back(match);
            pos += keyword.length();
        }
    }
    
    // Ищем сайты из чёрного списка
    for (const auto& site : m_blacklistSites) {
        std::wstring lowerSite = toLower(site);
        size_t pos = 0;
        
        while ((pos = lowerContent.find(lowerSite, pos)) != std::wstring::npos) {
            BrowserMatch match;
            match.browserType = browserType;
            match.source = filePath;
            match.matchType = L"url";
            match.matchedValue = site;
            match.riskWeight = m_siteVisitWeight;
            
            // Извлекаем контекст (URL)
            size_t start = (pos > 50) ? pos - 50 : 0;
            size_t len = 100 + site.length();
            match.context = wContent.substr(start, len);
            
            // Очищаем контекст
            for (wchar_t& c : match.context) {
                if (c < 32 || c > 126) c = L' ';
            }
            
            // Проверяем на попытку авторизации
            match.isAuthAttempt = isAuthAttempt(match.context);
            
            results.push_back(match);
            pos += site.length();
        }
    }
    
    return results;
}

bool BrowserScanner::checkKeywords(const std::wstring& text, std::wstring& outKeyword) {
    std::wstring lowerText = toLower(text);
    
    for (const auto& keyword : m_keywords) {
        std::wstring lowerKeyword = toLower(keyword);
        if (lowerText.find(lowerKeyword) != std::wstring::npos) {
            outKeyword = keyword;
            return true;
        }
    }
    
    return false;
}

bool BrowserScanner::checkBlacklist(const std::wstring& url, std::wstring& outSite) {
    std::wstring lowerUrl = toLower(url);
    
    for (const auto& site : m_blacklistSites) {
        std::wstring lowerSite = toLower(site);
        if (lowerUrl.find(lowerSite) != std::wstring::npos) {
            outSite = site;
            return true;
        }
    }
    
    return false;
}

bool BrowserScanner::isAuthAttempt(const std::wstring& url) {
    std::wstring lowerUrl = toLower(url);
    
    // Ключевые слова, указывающие на авторизацию
    std::vector<std::wstring> authKeywords = {
        L"login", L"signin", L"sign-in", L"auth", L"authenticate",
        L"register", L"signup", L"sign-up", L"account", L"password"
    };
    
    for (const auto& keyword : authKeywords) {
        if (lowerUrl.find(keyword) != std::wstring::npos) {
            return true;
        }
    }
    
    return false;
}

std::wstring BrowserScanner::toLower(const std::wstring& str) {
    std::wstring result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::towlower);
    return result;
}

const std::vector<BrowserMatch>& BrowserScanner::getMatches() const {
    return m_matches;
}

size_t BrowserScanner::getMatchCount() const {
    return m_matches.size();
}

void BrowserScanner::clearResults() {
    m_matches.clear();
}

std::wstring BrowserScanner::browserTypeToString(BrowserType type) {
    switch (type) {
        case BrowserType::CHROME:       return L"Google Chrome";
        case BrowserType::FIREFOX:      return L"Mozilla Firefox";
        case BrowserType::EDGE:         return L"Microsoft Edge";
        case BrowserType::OPERA:        return L"Opera";
        case BrowserType::MY_ACTIVITY:  return L"My Activity";
        default:                        return L"Неизвестный браузер";
    }
}

std::vector<BrowserMatch> BrowserScanner::analyzeHistoryDb(const std::wstring& dbPath, 
                                                           BrowserType browserType) {
    // SQLite анализ (упрощённо - читаем как текст)
    return scanTextFile(dbPath, browserType);
}

} // namespace ForensicScanner
