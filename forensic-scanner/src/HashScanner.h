#pragma once

#include <string>
#include <vector>
#include <functional>
#include <atomic>
#include "Types.h"

namespace ForensicScanner {

class HashScanner {
public:
    HashScanner();
    ~HashScanner();
    
    // Запуск сканирования
    void Scan(std::vector<TriggerEvent>& outTriggers);
    
    // Сканирование конкретной директории
    void ScanDirectory(const std::string& directory, 
                       std::vector<TriggerEvent>& outTriggers);
    
    // Сканирование одного файла
    bool ScanFile(const std::string& filePath, TriggerEvent& outTrigger);
    
    // Прогресс
    float GetProgress() const { return m_progress; }
    std::string GetCurrentFile() const { return m_currentFile; }
    int GetScannedCount() const { return m_scannedCount; }
    int GetMatchCount() const { return m_matchCount; }
    
    // Остановка сканирования
    void Stop() { m_stopRequested = true; }
    bool IsStopped() const { return m_stopRequested; }
    
    // Callback для обновления прогресса
    void SetProgressCallback(std::function<void(float, const std::string&)> callback);

private:
    std::atomic<float> m_progress{0.0f};
    std::atomic<int> m_scannedCount{0};
    std::atomic<int> m_matchCount{0};
    std::atomic<bool> m_stopRequested{false};
    std::string m_currentFile;
    
    std::function<void(float, const std::string&)> m_progressCallback;
    
    void UpdateProgress(float progress, const std::string& currentFile);
};

} // namespace ForensicScanner
