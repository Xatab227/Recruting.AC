#ifndef BROWSERSCANNER_H
#define BROWSERSCANNER_H

#include <string>
#include <vector>
#include <functional>

class BrowserScanner {
public:
    BrowserScanner();
    
    void scanBrowserHistory(const std::string& browserPath, std::function<void(int, int)> progressCallback = nullptr);
    void scanMyActivity(const std::string& activityPath, std::function<void(int, int)> progressCallback = nullptr);
    
    int getMatchCount() const { return matchCount; }

private:
    int matchCount = 0;
    
    void checkKeywords(const std::string& content, const std::string& source);
    void checkDomains(const std::string& url, const std::string& source);
    std::string toLower(const std::string& str);
};

#endif // BROWSERSCANNER_H
