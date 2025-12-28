#pragma once

#include <string>
#include <vector>
#include <functional>
#include <atomic>
#include "Types.h"

namespace ForensicScanner {

// Типы браузеров
enum class BrowserType {
    Chrome,
    Firefox,
    Edge,
    Opera,
    Brave,
    Unknown
};

class BrowserScanner {
public:
    BrowserScanner();
    ~BrowserScanner();
    
    // Запуск сканирования
    void Scan(std::vector<TriggerEvent>& outTriggers);
    
    // Сканирование конкретного браузера
    void ScanBrowser(BrowserType browser, std::vector<TriggerEvent>& outTriggers);
    
    // Сканирование файла My Activity (Google Takeout)
    void ScanMyActivity(const std::string& filePath, 
                        std::vector<TriggerEvent>& outTriggers);
    
    // Сканирование файла истории браузера
    void ScanHistoryFile(const std::string& filePath, 
                         BrowserType browser,
                         std::vector<TriggerEvent>& outTriggers);
    
    // Сканирование экспортированной истории (txt/html/json)
    void ScanExportedHistory(const std::string& filePath,
                             std::vector<TriggerEvent>& outTriggers);
    
    // Прогресс
    float GetProgress() const { return m_progress; }
    std::string GetCurrentBrowser() const { return m_currentBrowser; }
    int GetMatchCount() const { return m_matchCount; }
    
    // Остановка
    void Stop() { m_stopRequested = true; }
    bool IsStopped() const { return m_stopRequested; }
    
    // Callback
    void SetProgressCallback(std::function<void(float, const std::string&)> callback);
    
    // Имя браузера
    static std::string BrowserTypeToString(BrowserType browser);

private:
    std::atomic<float> m_progress{0.0f};
    std::atomic<int> m_matchCount{0};
    std::atomic<bool> m_stopRequested{false};
    std::string m_currentBrowser;
    
    std::function<void(float, const std::string&)> m_progressCallback;
    
    void UpdateProgress(float progress, const std::string& browser);
    
    // Получение пути к истории браузера
    std::string GetHistoryPath(BrowserType browser);
    
    // Парсинг SQLite (Chrome/Edge/Opera)
    void ParseChromiumHistory(const std::string& dbPath, 
                              BrowserType browser,
                              std::vector<TriggerEvent>& outTriggers);
    
    // Парсинг Firefox history
    void ParseFirefoxHistory(const std::string& dbPath,
                             std::vector<TriggerEvent>& outTriggers);
    
    // Анализ текстового/HTML контента
    void AnalyzeTextContent(const std::string& content,
                            const std::string& source,
                            std::vector<TriggerEvent>& outTriggers);
    
    // Анализ JSON контента
    void AnalyzeJsonContent(const std::string& content,
                            const std::string& source,
                            std::vector<TriggerEvent>& outTriggers);
    
    // Детектирование попыток авторизации
    bool DetectAuthAttempt(const std::string& url);
};

} // namespace ForensicScanner
