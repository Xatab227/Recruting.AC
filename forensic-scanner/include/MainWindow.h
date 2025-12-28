/**
 * @file MainWindow.h
 * @brief Главное окно приложения с GUI на ImGui
 */

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <Windows.h>
#include <d3d11.h>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>

#include "Config.h"
#include "Logger.h"
#include "HashScanner.h"
#include "BrowserScanner.h"
#include "DiscordScanner.h"
#include "RiskCalculator.h"

namespace ForensicScanner {

/**
 * @struct ScanProgress
 * @brief Структура для отслеживания прогресса сканирования
 */
struct ScanProgress {
    std::atomic<bool> isRunning;
    std::atomic<int> progress;
    std::wstring currentTask;
    std::mutex taskMutex;
    
    ScanProgress() : isRunning(false), progress(0) {}
};

/**
 * @class MainWindow
 * @brief Главное окно приложения
 */
class MainWindow {
public:
    /**
     * @brief Конструктор
     */
    MainWindow();
    
    /**
     * @brief Деструктор
     */
    ~MainWindow();
    
    /**
     * @brief Инициализировать окно
     * @param width Ширина окна
     * @param height Высота окна
     * @return true если успешно
     */
    bool initialize(int width = 1280, int height = 720);
    
    /**
     * @brief Запустить главный цикл
     * @return Код завершения
     */
    int run();
    
    /**
     * @brief Закрыть окно
     */
    void shutdown();

private:
    // Win32
    HWND m_hwnd;
    WNDCLASSEXW m_wc;
    
    // DirectX 11
    ID3D11Device* m_pd3dDevice;
    ID3D11DeviceContext* m_pd3dDeviceContext;
    IDXGISwapChain* m_pSwapChain;
    ID3D11RenderTargetView* m_mainRenderTargetView;
    
    // Модули сканера
    Config m_config;
    HashScanner m_hashScanner;
    BrowserScanner m_browserScanner;
    DiscordScanner m_discordScanner;
    RiskCalculator m_riskCalculator;
    
    // Прогресс сканирования
    ScanProgress m_scanProgress;
    std::thread m_scanThread;
    
    // Результаты
    RiskSummary m_riskSummary;
    bool m_scanComplete;
    
    // Вкладки логов
    int m_currentTab;
    
    // Размеры окна
    int m_width;
    int m_height;
    
    /**
     * @brief Создать DirectX устройство
     * @return true если успешно
     */
    bool createDeviceD3D();
    
    /**
     * @brief Очистить DirectX устройство
     */
    void cleanupDeviceD3D();
    
    /**
     * @brief Создать RenderTarget
     */
    void createRenderTarget();
    
    /**
     * @brief Очистить RenderTarget
     */
    void cleanupRenderTarget();
    
    /**
     * @brief Обработчик сообщений окна
     */
    static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    
    /**
     * @brief Отрисовать UI
     */
    void renderUI();
    
    /**
     * @brief Отрисовать главный экран
     */
    void renderMainScreen();
    
    /**
     * @brief Отрисовать индикатор риска
     */
    void renderRiskIndicator();
    
    /**
     * @brief Отрисовать сводку
     */
    void renderSummary();
    
    /**
     * @brief Отрисовать вкладки логов
     */
    void renderLogTabs();
    
    /**
     * @brief Отрисовать лог хешей
     */
    void renderHashLog();
    
    /**
     * @brief Отрисовать лог браузера
     */
    void renderBrowserLog();
    
    /**
     * @brief Отрисовать лог Discord
     */
    void renderDiscordLog();
    
    /**
     * @brief Отрисовать запись лога
     * @param entry Запись
     * @param index Индекс
     */
    void renderLogEntry(LogEntry& entry, int index);
    
    /**
     * @brief Запустить сканирование
     */
    void startScan();
    
    /**
     * @brief Процедура сканирования (в отдельном потоке)
     */
    void scanProcedure();
    
    /**
     * @brief Обновить текущую задачу
     * @param task Описание задачи
     */
    void updateTask(const std::wstring& task);
    
    /**
     * @brief Получить текущую задачу
     * @return Описание задачи
     */
    std::wstring getCurrentTask();
    
    /**
     * @brief Настроить стили ImGui
     */
    void setupImGuiStyle();
    
    /**
     * @brief Конвертировать wstring в string (UTF-8)
     */
    std::string wstringToUtf8(const std::wstring& wstr);
    
    /**
     * @brief Конвертировать string в wstring
     */
    std::wstring utf8ToWstring(const std::string& str);
};

} // namespace ForensicScanner

#endif // MAIN_WINDOW_H
