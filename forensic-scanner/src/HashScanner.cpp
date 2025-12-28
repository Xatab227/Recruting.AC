/**
 * @file HashScanner.cpp
 * @brief Реализация модуля анализа хешей
 */

#include "../include/HashScanner.h"
#include "../include/Logger.h"
#include "../include/Config.h"
#include <Windows.h>
#include <wincrypt.h>
#include <fstream>
#include <sstream>
#include <algorithm>

#pragma comment(lib, "advapi32.lib")

namespace ForensicScanner {

HashScanner::HashScanner() {
}

HashScanner::~HashScanner() {
}

bool HashScanner::loadHashDatabase(const std::wstring& databasePath) {
    std::wstring expandedPath = Config::expandEnvironmentPath(databasePath);
    std::wifstream file(expandedPath);
    
    if (!file.is_open()) {
        Logger::getInstance().warning(LogCategory::HASH, 
            L"Не удалось открыть базу хешей: " + expandedPath);
        return false;
    }
    
    m_hashDatabase.clear();
    std::wstring line;
    int lineNum = 0;
    
    while (std::getline(file, line)) {
        lineNum++;
        
        // Пропускаем пустые строки и комментарии
        if (line.empty() || line[0] == L'#') {
            continue;
        }
        
        // Формат: hash|category|description|weight
        std::wistringstream iss(line);
        std::wstring hash, category, description, weightStr;
        
        if (std::getline(iss, hash, L'|') &&
            std::getline(iss, category, L'|') &&
            std::getline(iss, description, L'|') &&
            std::getline(iss, weightStr)) {
            
            HashEntry entry;
            entry.hash = hash;
            entry.category = category;
            entry.description = description;
            
            try {
                entry.riskWeight = std::stoi(weightStr);
            } catch (...) {
                entry.riskWeight = 40;
            }
            
            // Приводим хеш к нижнему регистру для сравнения
            std::transform(entry.hash.begin(), entry.hash.end(), entry.hash.begin(), ::tolower);
            
            m_hashDatabase[entry.hash] = entry;
        }
    }
    
    file.close();
    
    Logger::getInstance().info(LogCategory::HASH, 
        L"Загружено " + std::to_wstring(m_hashDatabase.size()) + L" хешей из базы");
    
    return true;
}

std::vector<ScanResult> HashScanner::scanDirectory(const std::wstring& directory, 
                                                    bool recursive,
                                                    ScanProgressCallback callback) {
    std::wstring expandedPath = Config::expandEnvironmentPath(directory);
    std::vector<std::wstring> files = getFilesInDirectory(expandedPath, recursive);
    
    Logger::getInstance().info(LogCategory::HASH, 
        L"Сканирование директории: " + expandedPath + L" (" + std::to_wstring(files.size()) + L" файлов)");
    
    int total = static_cast<int>(files.size());
    int current = 0;
    
    for (const auto& filePath : files) {
        current++;
        
        if (callback) {
            // Извлекаем имя файла для отображения
            size_t lastSlash = filePath.find_last_of(L"\\/");
            std::wstring fileName = (lastSlash != std::wstring::npos) ? 
                                    filePath.substr(lastSlash + 1) : filePath;
            callback(current, total, fileName);
        }
        
        ScanResult result = scanFile(filePath);
        m_results.push_back(result);
        
        if (result.matched) {
            // Добавляем в лог
            LogEntry logEntry;
            logEntry.category = LogCategory::HASH;
            logEntry.triggerType = TriggerType::HASH_MATCH;
            logEntry.source = result.filePath;
            logEntry.matchedValue = result.hash;
            logEntry.details = L"Категория: " + result.matchedEntry.category + 
                              L", Описание: " + result.matchedEntry.description;
            logEntry.riskWeight = result.matchedEntry.riskWeight;
            
            Logger::getInstance().addEntry(logEntry);
        }
    }
    
    return m_results;
}

ScanResult HashScanner::scanFile(const std::wstring& filePath) {
    ScanResult result;
    result.filePath = filePath;
    
    // Извлекаем имя файла
    size_t lastSlash = filePath.find_last_of(L"\\/");
    result.fileName = (lastSlash != std::wstring::npos) ? 
                      filePath.substr(lastSlash + 1) : filePath;
    
    // Вычисляем хеш
    result.hash = calculateSHA256(filePath);
    
    if (!result.hash.empty()) {
        // Проверяем по базе
        result.matched = checkHash(result.hash, result.matchedEntry);
    }
    
    return result;
}

std::wstring HashScanner::calculateSHA256(const std::wstring& filePath) {
    HCRYPTPROV hProv = 0;
    HCRYPTHASH hHash = 0;
    
    // Открываем файл
    HANDLE hFile = CreateFileW(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ,
                               NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    
    if (hFile == INVALID_HANDLE_VALUE) {
        return L"";
    }
    
    // Инициализируем криптопровайдер
    if (!CryptAcquireContextW(&hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) {
        CloseHandle(hFile);
        return L"";
    }
    
    // Создаём хеш-объект
    if (!CryptCreateHash(hProv, CALG_SHA_256, 0, 0, &hHash)) {
        CryptReleaseContext(hProv, 0);
        CloseHandle(hFile);
        return L"";
    }
    
    // Читаем файл и вычисляем хеш
    BYTE buffer[8192];
    DWORD bytesRead = 0;
    
    while (ReadFile(hFile, buffer, sizeof(buffer), &bytesRead, NULL) && bytesRead > 0) {
        if (!CryptHashData(hHash, buffer, bytesRead, 0)) {
            CryptDestroyHash(hHash);
            CryptReleaseContext(hProv, 0);
            CloseHandle(hFile);
            return L"";
        }
    }
    
    // Получаем хеш
    BYTE hashValue[32];
    DWORD hashLen = 32;
    
    if (!CryptGetHashParam(hHash, HP_HASHVAL, hashValue, &hashLen, 0)) {
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        CloseHandle(hFile);
        return L"";
    }
    
    // Преобразуем в строку
    std::wstring hashString = bytesToHexString(hashValue, hashLen);
    
    // Освобождаем ресурсы
    CryptDestroyHash(hHash);
    CryptReleaseContext(hProv, 0);
    CloseHandle(hFile);
    
    return hashString;
}

bool HashScanner::checkHash(const std::wstring& hash, HashEntry& outEntry) {
    // Приводим к нижнему регистру
    std::wstring lowerHash = hash;
    std::transform(lowerHash.begin(), lowerHash.end(), lowerHash.begin(), ::tolower);
    
    auto it = m_hashDatabase.find(lowerHash);
    if (it != m_hashDatabase.end()) {
        outEntry = it->second;
        return true;
    }
    
    return false;
}

size_t HashScanner::getDatabaseSize() const {
    return m_hashDatabase.size();
}

std::map<std::wstring, int> HashScanner::getCategoryStats() const {
    std::map<std::wstring, int> stats;
    
    for (const auto& pair : m_hashDatabase) {
        stats[pair.second.category]++;
    }
    
    return stats;
}

std::vector<ScanResult> HashScanner::getMatches() const {
    std::vector<ScanResult> matches;
    
    for (const auto& result : m_results) {
        if (result.matched) {
            matches.push_back(result);
        }
    }
    
    return matches;
}

void HashScanner::clearResults() {
    m_results.clear();
}

std::vector<std::wstring> HashScanner::getFilesInDirectory(const std::wstring& directory, bool recursive) {
    std::vector<std::wstring> files;
    
    std::wstring searchPath = directory;
    if (searchPath.back() != L'\\' && searchPath.back() != L'/') {
        searchPath += L'\\';
    }
    searchPath += L'*';
    
    WIN32_FIND_DATAW findData;
    HANDLE hFind = FindFirstFileW(searchPath.c_str(), &findData);
    
    if (hFind == INVALID_HANDLE_VALUE) {
        return files;
    }
    
    do {
        std::wstring fileName = findData.cFileName;
        
        // Пропускаем . и ..
        if (fileName == L"." || fileName == L"..") {
            continue;
        }
        
        std::wstring fullPath = directory;
        if (fullPath.back() != L'\\' && fullPath.back() != L'/') {
            fullPath += L'\\';
        }
        fullPath += fileName;
        
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            // Рекурсивно обрабатываем поддиректории
            if (recursive) {
                std::vector<std::wstring> subFiles = getFilesInDirectory(fullPath, true);
                files.insert(files.end(), subFiles.begin(), subFiles.end());
            }
        } else {
            files.push_back(fullPath);
        }
    } while (FindNextFileW(hFind, &findData));
    
    FindClose(hFind);
    
    return files;
}

std::wstring HashScanner::bytesToHexString(const unsigned char* bytes, size_t length) {
    static const wchar_t hexChars[] = L"0123456789abcdef";
    std::wstring result;
    result.reserve(length * 2);
    
    for (size_t i = 0; i < length; i++) {
        result += hexChars[(bytes[i] >> 4) & 0x0F];
        result += hexChars[bytes[i] & 0x0F];
    }
    
    return result;
}

} // namespace ForensicScanner
