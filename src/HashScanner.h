#ifndef HASHSCANNER_H
#define HASHSCANNER_H

#include <string>
#include <vector>
#include <set>
#include <functional>

class HashScanner {
public:
    HashScanner();
    
    bool loadHashDatabase(const std::string& path);
    void scanDirectory(const std::string& directory, std::function<void(int, int)> progressCallback = nullptr);
    std::string calculateSHA256(const std::string& filePath);
    
    int getScannedCount() const { return scannedCount; }
    int getMatchCount() const { return matchCount; }

private:
    std::set<std::string> hashDatabase;
    int scannedCount = 0;
    int matchCount = 0;
    
    void scanFile(const std::string& filePath);
};

#endif // HASHSCANNER_H
