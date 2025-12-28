#include "HashScanner.h"
#include "Config.h"
#include "Logger.h"
#include "Utils.h"
#include <filesystem>

namespace fs = std::filesystem;

namespace ForensicScanner {

HashScanner::HashScanner() {
    m_progress = 0.0f;
    m_scannedCount = 0;
    m_matchCount = 0;
    m_stopRequested = false;
}

HashScanner::~HashScanner() {
    Stop();
}

void HashScanner::Scan(std::vector<TriggerEvent>& outTriggers) {
    Logger::Instance().Info("Начало сканирования хешей...", "hash");
    
    m_progress = 0.0f;
    m_scannedCount = 0;
    m_matchCount = 0;
    m_stopRequested = false;
    
    const auto& scanPaths = Config::Instance().GetScanPaths();
    
    // Также сканируем Recent и Prefetch
    std::vector<std::string> allPaths = scanPaths;
    allPaths.push_back(Config::Instance().GetRecentPath());
    allPaths.push_back(Config::Instance().GetPrefetchPath());
    
    // Удаляем дубликаты
    std::sort(allPaths.begin(), allPaths.end());
    allPaths.erase(std::unique(allPaths.begin(), allPaths.end()), allPaths.end());
    
    int totalPaths = static_cast<int>(allPaths.size());
    int currentPath = 0;
    
    for (const auto& path : allPaths) {
        if (m_stopRequested) break;
        
        std::string expandedPath = Utils::ExpandEnvironmentPath(path);
        
        if (Utils::DirectoryExists(expandedPath)) {
            Logger::Instance().Info("Сканирование директории: " + expandedPath, "hash");
            ScanDirectory(expandedPath, outTriggers);
        } else {
            Logger::Instance().Warning("Директория не найдена: " + expandedPath, "hash");
        }
        
        currentPath++;
        m_progress = static_cast<float>(currentPath) / static_cast<float>(totalPaths);
        UpdateProgress(m_progress, "");
    }
    
    Logger::Instance().Info(
        "Сканирование хешей завершено. Проверено файлов: " + 
        std::to_string(m_scannedCount) + ", совпадений: " + 
        std::to_string(m_matchCount), "hash");
    
    m_progress = 1.0f;
}

void HashScanner::ScanDirectory(const std::string& directory, 
                                 std::vector<TriggerEvent>& outTriggers) {
    try {
        auto files = Utils::GetFilesInDirectory(directory, false);
        int totalFiles = static_cast<int>(files.size());
        int currentFile = 0;
        
        for (const auto& filePath : files) {
            if (m_stopRequested) break;
            
            m_currentFile = Utils::GetFileName(filePath);
            UpdateProgress(m_progress, m_currentFile);
            
            TriggerEvent trigger;
            if (ScanFile(filePath, trigger)) {
                outTriggers.push_back(trigger);
                m_matchCount++;
                
                // Логируем триггер
                Logger::Instance().LogTrigger(trigger);
            }
            
            m_scannedCount++;
            currentFile++;
        }
    } catch (const std::exception& e) {
        Logger::Instance().Error(
            "Ошибка при сканировании директории " + directory + ": " + e.what(), 
            "hash");
    }
}

bool HashScanner::ScanFile(const std::string& filePath, TriggerEvent& outTrigger) {
    // Пропускаем очень большие файлы (> 100 МБ)
    try {
        auto fileSize = fs::file_size(filePath);
        if (fileSize > 100 * 1024 * 1024) {
            return false;
        }
    } catch (...) {
        return false;
    }
    
    // Вычисляем хеш файла
    std::string hash = Utils::CalculateSHA256(filePath);
    if (hash.empty()) {
        return false;
    }
    
    // Проверяем в базе
    HashEntry entry;
    if (Config::Instance().FindHash(hash, entry)) {
        outTrigger.category = TriggerCategory::Hash;
        outTrigger.type = TriggerType::HashMatch;
        outTrigger.source = filePath;
        outTrigger.matchedValue = hash;
        outTrigger.details = "Категория: " + entry.category + ", Название: " + entry.name;
        outTrigger.weight = entry.weight;
        outTrigger.timestamp = Utils::GetCurrentTimestamp();
        outTrigger.context = "Файл: " + Utils::GetFileName(filePath);
        
        return true;
    }
    
    // Дополнительная проверка по имени файла на подозрительные паттерны
    std::string fileName = Utils::ToLower(Utils::GetFileName(filePath));
    std::string matchedKeyword;
    
    if (Config::Instance().ContainsKeyword(fileName, matchedKeyword)) {
        outTrigger.category = TriggerCategory::Hash;
        outTrigger.type = TriggerType::KeywordFound;
        outTrigger.source = filePath;
        outTrigger.matchedValue = matchedKeyword;
        outTrigger.details = "Подозрительное имя файла";
        outTrigger.weight = Config::Instance().GetWeights().keywordFound;
        outTrigger.timestamp = Utils::GetCurrentTimestamp();
        outTrigger.context = "Имя файла: " + fileName;
        
        return true;
    }
    
    return false;
}

void HashScanner::SetProgressCallback(
    std::function<void(float, const std::string&)> callback) {
    m_progressCallback = callback;
}

void HashScanner::UpdateProgress(float progress, const std::string& currentFile) {
    if (m_progressCallback) {
        m_progressCallback(progress, currentFile);
    }
}

} // namespace ForensicScanner
