#ifndef DISCORDSCANNER_H
#define DISCORDSCANNER_H

#include <string>
#include <vector>
#include <functional>

class DiscordScanner {
public:
    DiscordScanner();
    
    void scanDiscordFiles(const std::string& discordPath, std::function<void(int, int)> progressCallback = nullptr);
    void scanDiscordLogs(const std::string& logPath, std::function<void(int, int)> progressCallback = nullptr);
    
    int getMatchCount() const { return matchCount; }

private:
    int matchCount = 0;
    
    void checkKeywords(const std::string& content, const std::string& source);
    void checkServers(const std::string& content, const std::string& source);
    std::string toLower(const std::string& str);
};

#endif // DISCORDSCANNER_H
