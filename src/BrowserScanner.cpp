#include "BrowserScanner.h"
#include "Logger.h"
#include "Config.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <regex>
#include <filesystem>

BrowserScanner::BrowserScanner() {
}

std::string BrowserScanner::toLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

void BrowserScanner::checkKeywords(const std::string& content, const std::string& source) {
    Config& config = Config::getInstance();
    std::string lowerContent = toLower(content);
    
    for (const auto& keyword : config.getKeywords()) {
        std::string lowerKeyword = toLower(keyword);
        if (lowerContent.find(lowerKeyword) != std::string::npos) {
            matchCount++;
            Logger::getInstance().log(
                LogType::BROWSER,
                "Keyword Match",
                source,
                keyword,
                config.getKeywordWeight()
            );
        }
    }
}

void BrowserScanner::checkDomains(const std::string& url, const std::string& source) {
    Config& config = Config::getInstance();
    std::string lowerUrl = toLower(url);
    
    for (const auto& domain : config.getSuspiciousDomains()) {
        std::string lowerDomain = toLower(domain);
        if (lowerUrl.find(lowerDomain) != std::string::npos) {
            matchCount++;
            Logger::getInstance().log(
                LogType::BROWSER,
                "Domain Match",
                source,
                domain,
                config.getDomainWeight()
            );
        }
    }
}

void BrowserScanner::scanBrowserHistory(const std::string& browserPath, std::function<void(int, int)> progressCallback) {
    if (!std::filesystem::exists(browserPath)) {
        return;
    }
    
    // Для упрощения читаем как текстовый файл
    // В реальности нужно использовать SQLite для Chrome/Edge
    std::ifstream file(browserPath, std::ios::binary);
    if (!file.is_open()) {
        return;
    }
    
    std::string line;
    int lineCount = 0;
    while (std::getline(file, line)) {
        lineCount++;
        checkKeywords(line, browserPath);
        checkDomains(line, browserPath);
        
        if (progressCallback && lineCount % 100 == 0) {
            progressCallback(lineCount, lineCount);
        }
    }
    file.close();
}

void BrowserScanner::scanMyActivity(const std::string& activityPath, std::function<void(int, int)> progressCallback) {
    if (!std::filesystem::exists(activityPath)) {
        return;
    }
    
    std::ifstream file(activityPath);
    if (!file.is_open()) {
        return;
    }
    
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    
    // Поиск URL в JSON/HTML
    std::regex urlRegex(R"((https?://[^\s"<>]+))");
    std::sregex_iterator iter(content.begin(), content.end(), urlRegex);
    std::sregex_iterator end;
    
    int urlCount = 0;
    for (; iter != end; ++iter) {
        std::string url = iter->str();
        checkDomains(url, activityPath);
        urlCount++;
        
        if (progressCallback && urlCount % 10 == 0) {
            progressCallback(urlCount, urlCount);
        }
    }
    
    checkKeywords(content, activityPath);
}
