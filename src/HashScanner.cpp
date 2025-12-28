#include "HashScanner.h"
#include "Logger.h"
#include "Config.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <windows.h>
#include <wincrypt.h>

#pragma comment(lib, "advapi32.lib")

HashScanner::HashScanner() {
}

bool HashScanner::loadHashDatabase(const std::string& path) {
    hashDatabase.clear();
    std::ifstream file(path);
    if (!file.is_open()) {
        return false;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty() && line[0] != '#') {
            // Удалить пробелы и привести к нижнему регистру
            std::string hash = line;
            hash.erase(std::remove_if(hash.begin(), hash.end(), ::isspace), hash.end());
            std::transform(hash.begin(), hash.end(), hash.begin(), ::tolower);
            if (!hash.empty()) {
                hashDatabase.insert(hash);
            }
        }
    }
    file.close();
    return true;
}

std::string HashScanner::calculateSHA256(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        return "";
    }
    
    HCRYPTPROV hProv = 0;
    HCRYPTHASH hHash = 0;
    
    if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) {
        file.close();
        return "";
    }
    
    if (!CryptCreateHash(hProv, CALG_SHA_256, 0, 0, &hHash)) {
        CryptReleaseContext(hProv, 0);
        file.close();
        return "";
    }
    
    const size_t bufferSize = 8192;
    char buffer[bufferSize];
    
    while (file.read(buffer, bufferSize) || file.gcount() > 0) {
        if (!CryptHashData(hHash, (BYTE*)buffer, file.gcount(), 0)) {
            CryptDestroyHash(hHash);
            CryptReleaseContext(hProv, 0);
            file.close();
            return "";
        }
    }
    
    file.close();
    
    DWORD hashLen = 32;
    BYTE hash[32];
    if (!CryptGetHashParam(hHash, HP_HASHVAL, hash, &hashLen, 0)) {
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        return "";
    }
    
    CryptDestroyHash(hHash);
    CryptReleaseContext(hProv, 0);
    
    std::ostringstream oss;
    for (DWORD i = 0; i < hashLen; i++) {
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    
    return oss.str();
}

void HashScanner::scanDirectory(const std::string& directory, std::function<void(int, int)> progressCallback) {
    if (!std::filesystem::exists(directory)) {
        return;
    }
    
    int totalFiles = 0;
    int processedFiles = 0;
    
    // Подсчет файлов
    for (const auto& entry : std::filesystem::recursive_directory_iterator(directory)) {
        if (entry.is_regular_file()) {
            totalFiles++;
        }
    }
    
    // Сканирование
    for (const auto& entry : std::filesystem::recursive_directory_iterator(directory)) {
        if (entry.is_regular_file()) {
            scanFile(entry.path().string());
            processedFiles++;
            
            if (progressCallback) {
                progressCallback(processedFiles, totalFiles);
            }
        }
    }
}

void HashScanner::scanFile(const std::string& filePath) {
    scannedCount++;
    
    try {
        std::string hash = calculateSHA256(filePath);
        if (hash.empty()) {
            return;
        }
        
        std::transform(hash.begin(), hash.end(), hash.begin(), ::tolower);
        
        if (hashDatabase.find(hash) != hashDatabase.end()) {
            matchCount++;
            Config& config = Config::getInstance();
            Logger::getInstance().log(
                LogType::HASH,
                "Hash Match",
                filePath,
                hash,
                config.getHashMatchWeight()
            );
        }
    } catch (...) {
        // Игнорируем ошибки при сканировании отдельных файлов
    }
}
