#pragma once

#include <string>
#include <vector>
#include <functional>
#include <atomic>
#include "Types.h"

namespace ForensicScanner {

class DiscordScanner {
public:
    DiscordScanner();
    ~DiscordScanner();
    
    // Запуск сканирования
    void Scan(std::vector<TriggerEvent>& outTriggers);
    
    // Сканирование локальных данных Discord
    void ScanLocalData(std::vector<TriggerEvent>& outTriggers);
    
    // Сканирование экспортированных логов
    void ScanExportedLogs(const std::string& filePath,
                          std::vector<TriggerEvent>& outTriggers);
    
    // Сканирование LevelDB (локальное хранилище Discord)
    void ScanLevelDB(const std::string& dbPath,
                     std::vector<TriggerEvent>& outTriggers);
    
    // Сканирование кеша Discord
    void ScanCache(const std::string& cachePath,
                   std::vector<TriggerEvent>& outTriggers);
    
    // Прогресс
    float GetProgress() const { return m_progress; }
    std::string GetCurrentOperation() const { return m_currentOperation; }
    int GetMatchCount() const { return m_matchCount; }
    
    // Остановка
    void Stop() { m_stopRequested = true; }
    bool IsStopped() const { return m_stopRequested; }
    
    // Callback
    void SetProgressCallback(std::function<void(float, const std::string&)> callback);

private:
    std::atomic<float> m_progress{0.0f};
    std::atomic<int> m_matchCount{0};
    std::atomic<bool> m_stopRequested{false};
    std::string m_currentOperation;
    
    std::function<void(float, const std::string&)> m_progressCallback;
    
    void UpdateProgress(float progress, const std::string& operation);
    
    // Получение путей к данным Discord
    std::vector<std::string> GetDiscordDataPaths();
    
    // Анализ содержимого файла
    void AnalyzeFileContent(const std::string& content,
                            const std::string& source,
                            std::vector<TriggerEvent>& outTriggers);
    
    // Поиск названий серверов/каналов
    void SearchForServerNames(const std::string& content,
                              const std::string& source,
                              std::vector<TriggerEvent>& outTriggers);
    
    // Поиск ключевых слов в сообщениях
    void SearchForKeywords(const std::string& content,
                           const std::string& source,
                           std::vector<TriggerEvent>& outTriggers);
    
    // Извлечение контекста вокруг найденного совпадения
    std::string ExtractContext(const std::string& content, 
                               size_t matchPos, 
                               size_t contextLength = 100);
};

} // namespace ForensicScanner
