#include "BrowserScanner.h"
#include "Config.h"
#include "Logger.h"
#include "Utils.h"
#include <fstream>
#include <sstream>
#include <regex>

namespace ForensicScanner {

BrowserScanner::BrowserScanner() {
    m_progress = 0.0f;
    m_matchCount = 0;
    m_stopRequested = false;
}

BrowserScanner::~BrowserScanner() {
    Stop();
}

void BrowserScanner::Scan(std::vector<TriggerEvent>& outTriggers) {
    Logger::Instance().Info("Начало сканирования браузеров...", "browser");
    
    m_progress = 0.0f;
    m_matchCount = 0;
    m_stopRequested = false;
    
    std::vector<BrowserType> browsers = {
        BrowserType::Chrome,
        BrowserType::Edge,
        BrowserType::Firefox,
        BrowserType::Opera,
        BrowserType::Brave
    };
    
    int totalBrowsers = static_cast<int>(browsers.size());
    int currentBrowser = 0;
    
    for (auto browser : browsers) {
        if (m_stopRequested) break;
        
        m_currentBrowser = BrowserTypeToString(browser);
        UpdateProgress(m_progress, m_currentBrowser);
        
        Logger::Instance().Info("Сканирование: " + m_currentBrowser, "browser");
        ScanBrowser(browser, outTriggers);
        
        currentBrowser++;
        m_progress = static_cast<float>(currentBrowser) / static_cast<float>(totalBrowsers);
    }
    
    Logger::Instance().Info(
        "Сканирование браузеров завершено. Найдено совпадений: " + 
        std::to_string(m_matchCount), "browser");
    
    m_progress = 1.0f;
}

void BrowserScanner::ScanBrowser(BrowserType browser, 
                                  std::vector<TriggerEvent>& outTriggers) {
    std::string historyPath = GetHistoryPath(browser);
    
    if (historyPath.empty() || !Utils::FileExists(historyPath)) {
        Logger::Instance().Debug(
            "История не найдена для " + BrowserTypeToString(browser), 
            "browser");
        return;
    }
    
    ScanHistoryFile(historyPath, browser, outTriggers);
}

void BrowserScanner::ScanMyActivity(const std::string& filePath, 
                                     std::vector<TriggerEvent>& outTriggers) {
    if (!Utils::FileExists(filePath)) {
        Logger::Instance().Warning("Файл My Activity не найден: " + filePath, "browser");
        return;
    }
    
    Logger::Instance().Info("Анализ My Activity: " + filePath, "browser");
    
    std::string content = Utils::ReadFileContent(filePath);
    if (content.empty()) {
        return;
    }
    
    // Определяем формат файла
    std::string ext = Utils::ToLower(Utils::GetExtension(filePath));
    
    if (ext == ".json") {
        AnalyzeJsonContent(content, filePath, outTriggers);
    } else {
        AnalyzeTextContent(content, filePath, outTriggers);
    }
}

void BrowserScanner::ScanHistoryFile(const std::string& filePath, 
                                      BrowserType browser,
                                      std::vector<TriggerEvent>& outTriggers) {
    // Для Chromium-based браузеров (Chrome, Edge, Opera, Brave)
    // история хранится в SQLite базе данных
    // Для упрощения мы читаем файл как бинарный и ищем URL-паттерны
    
    std::string content = Utils::ReadFileContent(filePath);
    if (content.empty()) {
        return;
    }
    
    // Ищем URL-паттерны в бинарных данных
    std::regex urlPattern(R"(https?://[a-zA-Z0-9\-\.]+\.[a-zA-Z]{2,}[^\s\x00-\x1F]*)");
    std::smatch match;
    
    std::string::const_iterator searchStart(content.cbegin());
    while (std::regex_search(searchStart, content.cend(), match, urlPattern)) {
        if (m_stopRequested) break;
        
        std::string url = match[0].str();
        
        // Проверяем на черный список
        std::string matchedSite;
        if (Config::Instance().IsBlacklistedSite(url, matchedSite)) {
            TriggerEvent trigger;
            trigger.category = TriggerCategory::Browser;
            trigger.type = TriggerType::SuspiciousURL;
            trigger.source = filePath;
            trigger.matchedValue = url;
            trigger.details = "Браузер: " + BrowserTypeToString(browser);
            trigger.weight = Config::Instance().GetWeights().suspiciousURL;
            trigger.timestamp = Utils::GetCurrentTimestamp();
            trigger.context = "Совпадение с: " + matchedSite;
            
            outTriggers.push_back(trigger);
            m_matchCount++;
            
            Logger::Instance().LogTrigger(trigger);
        }
        
        // Проверяем на ключевые слова в URL
        std::string matchedKeyword;
        if (Config::Instance().ContainsKeyword(url, matchedKeyword)) {
            TriggerEvent trigger;
            trigger.category = TriggerCategory::Browser;
            trigger.type = TriggerType::KeywordFound;
            trigger.source = filePath;
            trigger.matchedValue = matchedKeyword;
            trigger.details = "Браузер: " + BrowserTypeToString(browser);
            trigger.weight = Config::Instance().GetWeights().keywordFound;
            trigger.timestamp = Utils::GetCurrentTimestamp();
            trigger.context = "URL: " + url.substr(0, 100);
            
            outTriggers.push_back(trigger);
            m_matchCount++;
            
            Logger::Instance().LogTrigger(trigger);
        }
        
        // Проверяем на попытку авторизации
        if (DetectAuthAttempt(url)) {
            std::string site;
            if (Config::Instance().IsBlacklistedSite(url, site)) {
                TriggerEvent trigger;
                trigger.category = TriggerCategory::Browser;
                trigger.type = TriggerType::AuthAttempt;
                trigger.source = filePath;
                trigger.matchedValue = url;
                trigger.details = "Возможная попытка авторизации на: " + site;
                trigger.weight = Config::Instance().GetWeights().authAttempt;
                trigger.timestamp = Utils::GetCurrentTimestamp();
                trigger.context = "Браузер: " + BrowserTypeToString(browser);
                
                outTriggers.push_back(trigger);
                m_matchCount++;
                
                Logger::Instance().LogTrigger(trigger);
            }
        }
        
        searchStart = match.suffix().first;
    }
}

void BrowserScanner::ScanExportedHistory(const std::string& filePath,
                                          std::vector<TriggerEvent>& outTriggers) {
    if (!Utils::FileExists(filePath)) {
        return;
    }
    
    std::string content = Utils::ReadFileContent(filePath);
    if (content.empty()) {
        return;
    }
    
    std::string ext = Utils::ToLower(Utils::GetExtension(filePath));
    
    if (ext == ".json") {
        AnalyzeJsonContent(content, filePath, outTriggers);
    } else {
        AnalyzeTextContent(content, filePath, outTriggers);
    }
}

std::string BrowserScanner::BrowserTypeToString(BrowserType browser) {
    switch (browser) {
        case BrowserType::Chrome:  return "Google Chrome";
        case BrowserType::Firefox: return "Mozilla Firefox";
        case BrowserType::Edge:    return "Microsoft Edge";
        case BrowserType::Opera:   return "Opera";
        case BrowserType::Brave:   return "Brave";
        default:                   return "Unknown";
    }
}

void BrowserScanner::SetProgressCallback(
    std::function<void(float, const std::string&)> callback) {
    m_progressCallback = callback;
}

void BrowserScanner::UpdateProgress(float progress, const std::string& browser) {
    if (m_progressCallback) {
        m_progressCallback(progress, browser);
    }
}

std::string BrowserScanner::GetHistoryPath(BrowserType browser) {
    std::string path;
    
    switch (browser) {
        case BrowserType::Chrome:
            path = Utils::ExpandEnvironmentPath(
                "%LOCALAPPDATA%\\Google\\Chrome\\User Data\\Default\\History");
            break;
        case BrowserType::Edge:
            path = Utils::ExpandEnvironmentPath(
                "%LOCALAPPDATA%\\Microsoft\\Edge\\User Data\\Default\\History");
            break;
        case BrowserType::Firefox:
            // Firefox использует другую структуру, ищем профиль
            {
                std::string profilesPath = Utils::ExpandEnvironmentPath(
                    "%APPDATA%\\Mozilla\\Firefox\\Profiles");
                if (Utils::DirectoryExists(profilesPath)) {
                    auto dirs = Utils::GetFilesInDirectory(profilesPath, false);
                    for (const auto& dir : dirs) {
                        std::string histPath = dir + "\\places.sqlite";
                        if (Utils::FileExists(histPath)) {
                            return histPath;
                        }
                    }
                }
            }
            break;
        case BrowserType::Opera:
            path = Utils::ExpandEnvironmentPath(
                "%APPDATA%\\Opera Software\\Opera Stable\\History");
            break;
        case BrowserType::Brave:
            path = Utils::ExpandEnvironmentPath(
                "%LOCALAPPDATA%\\BraveSoftware\\Brave-Browser\\User Data\\Default\\History");
            break;
        default:
            break;
    }
    
    return path;
}

void BrowserScanner::AnalyzeTextContent(const std::string& content,
                                         const std::string& source,
                                         std::vector<TriggerEvent>& outTriggers) {
    // Разбиваем на строки и анализируем каждую
    std::istringstream stream(content);
    std::string line;
    int lineNum = 0;
    
    while (std::getline(stream, line)) {
        if (m_stopRequested) break;
        lineNum++;
        
        // Проверяем на ключевые слова
        std::string matchedKeyword;
        if (Config::Instance().ContainsKeyword(line, matchedKeyword)) {
            TriggerEvent trigger;
            trigger.category = TriggerCategory::Browser;
            trigger.type = TriggerType::KeywordFound;
            trigger.source = source;
            trigger.matchedValue = matchedKeyword;
            trigger.details = "Строка: " + std::to_string(lineNum);
            trigger.weight = Config::Instance().GetWeights().keywordFound;
            trigger.timestamp = Utils::GetCurrentTimestamp();
            trigger.context = line.substr(0, 150);
            
            outTriggers.push_back(trigger);
            m_matchCount++;
            
            Logger::Instance().LogTrigger(trigger);
        }
        
        // Проверяем на URL из черного списка
        std::string matchedSite;
        if (Config::Instance().IsBlacklistedSite(line, matchedSite)) {
            TriggerEvent trigger;
            trigger.category = TriggerCategory::Browser;
            trigger.type = TriggerType::SuspiciousURL;
            trigger.source = source;
            trigger.matchedValue = matchedSite;
            trigger.details = "Строка: " + std::to_string(lineNum);
            trigger.weight = Config::Instance().GetWeights().suspiciousURL;
            trigger.timestamp = Utils::GetCurrentTimestamp();
            trigger.context = line.substr(0, 150);
            
            outTriggers.push_back(trigger);
            m_matchCount++;
            
            Logger::Instance().LogTrigger(trigger);
        }
    }
}

void BrowserScanner::AnalyzeJsonContent(const std::string& content,
                                         const std::string& source,
                                         std::vector<TriggerEvent>& outTriggers) {
    // Простой поиск по тексту JSON без полного парсинга
    // Ищем URL и ключевые слова
    
    // Ищем все значения в кавычках
    // Важно: используем raw-string с delimiter, чтобы последовательность )" не ломала литерал.
    std::regex valuePattern(R"re("([^"]+)")re");
    std::smatch match;
    
    std::string::const_iterator searchStart(content.cbegin());
    while (std::regex_search(searchStart, content.cend(), match, valuePattern)) {
        if (m_stopRequested) break;
        
        std::string value = match[1].str();
        
        // Проверяем на ключевые слова
        std::string matchedKeyword;
        if (Config::Instance().ContainsKeyword(value, matchedKeyword)) {
            TriggerEvent trigger;
            trigger.category = TriggerCategory::Browser;
            trigger.type = TriggerType::KeywordFound;
            trigger.source = source;
            trigger.matchedValue = matchedKeyword;
            trigger.details = "JSON файл";
            trigger.weight = Config::Instance().GetWeights().keywordFound;
            trigger.timestamp = Utils::GetCurrentTimestamp();
            trigger.context = value.substr(0, 150);
            
            outTriggers.push_back(trigger);
            m_matchCount++;
            
            Logger::Instance().LogTrigger(trigger);
        }
        
        // Проверяем на URL из черного списка
        std::string matchedSite;
        if (Config::Instance().IsBlacklistedSite(value, matchedSite)) {
            TriggerEvent trigger;
            trigger.category = TriggerCategory::Browser;
            trigger.type = TriggerType::SuspiciousURL;
            trigger.source = source;
            trigger.matchedValue = matchedSite;
            trigger.details = "JSON файл";
            trigger.weight = Config::Instance().GetWeights().suspiciousURL;
            trigger.timestamp = Utils::GetCurrentTimestamp();
            trigger.context = value.substr(0, 150);
            
            outTriggers.push_back(trigger);
            m_matchCount++;
            
            Logger::Instance().LogTrigger(trigger);
        }
        
        searchStart = match.suffix().first;
    }
}

bool BrowserScanner::DetectAuthAttempt(const std::string& url) {
    std::string lowerUrl = Utils::ToLower(url);
    
    // Паттерны авторизации
    std::vector<std::string> authPatterns = {
        "/login",
        "/signin",
        "/sign-in",
        "/auth",
        "/account",
        "/register",
        "/signup",
        "/sign-up",
        "oauth",
        "authenticate"
    };
    
    for (const auto& pattern : authPatterns) {
        if (Utils::Contains(lowerUrl, pattern, false)) {
            return true;
        }
    }
    
    return false;
}

} // namespace ForensicScanner
