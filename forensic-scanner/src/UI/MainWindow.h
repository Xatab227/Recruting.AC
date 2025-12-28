#pragma once

#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include "../Types.h"

// Forward declarations
struct ImVec4;

namespace ForensicScanner {

class MainWindow {
public:
    MainWindow();
    ~MainWindow();
    
    // Инициализация и запуск
    bool Initialize(const std::string& title, int width, int height);
    void Run();
    void Shutdown();
    
    // Проверка, должно ли окно закрыться
    bool ShouldClose() const { return m_shouldClose; }

private:
    // Win32 + DirectX11 инициализация
    bool CreateDeviceD3D();
    void CleanupDeviceD3D();
    void CreateRenderTarget();
    void CleanupRenderTarget();
    
    // Рендеринг
    void RenderFrame();
    void RenderUI();
    
    // UI компоненты
    void RenderMainPanel();
    void RenderRiskIndicator();
    void RenderSummary();
    void RenderLogTabs();
    void RenderHashLog();
    void RenderBrowserLog();
    void RenderDiscordLog();
    void RenderTriggerDetails(const TriggerEvent& trigger, int index);
    void RenderControlButtons();
    void RenderProgressBar();
    void RenderStatusBar();
    
    // Действия
    void StartScan();
    void StopScan();
    void ExportReport();
    void ClearResults();
    
    // Поток сканирования
    void ScanThread();
    
    // Вспомогательные
    void UpdateProgress(float progress, const std::string& message);
    
    // Состояние
    bool m_shouldClose = false;
    bool m_initialized = false;
    
    // Сканирование
    std::thread m_scanThread;
    std::atomic<bool> m_scanning{false};
    std::atomic<bool> m_stopScan{false};
    std::atomic<float> m_scanProgress{0.0f};
    std::string m_scanStatus;
    std::mutex m_statusMutex;
    
    // Результаты
    ScanResults m_results;
    std::mutex m_resultsMutex;
    
    // UI состояние
    int m_currentTab = 0;
    bool m_showDetails[1024] = {false}; // Развёрнутые элементы
    
    // Размеры окна
    int m_width = 1280;
    int m_height = 720;
    std::string m_title;
    
    // Win32 handles (определены как void* для совместимости)
    void* m_hwnd = nullptr;
    void* m_d3dDevice = nullptr;
    void* m_d3dContext = nullptr;
    void* m_swapChain = nullptr;
    void* m_renderTargetView = nullptr;
};

} // namespace ForensicScanner
