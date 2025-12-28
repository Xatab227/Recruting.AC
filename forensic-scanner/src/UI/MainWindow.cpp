/**
 * @file MainWindow.cpp
 * @brief Реализация главного окна приложения
 */

#include "../../include/MainWindow.h"

// ImGui
#include "../../libs/imgui/imgui.h"
#include "../../libs/imgui/imgui_impl_win32.h"
#include "../../libs/imgui/imgui_impl_dx11.h"

#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace ForensicScanner {

// Глобальный указатель на окно для WndProc
static MainWindow* g_mainWindow = nullptr;

MainWindow::MainWindow()
    : m_hwnd(nullptr)
    , m_pd3dDevice(nullptr)
    , m_pd3dDeviceContext(nullptr)
    , m_pSwapChain(nullptr)
    , m_mainRenderTargetView(nullptr)
    , m_currentTab(0)
    , m_width(1280)
    , m_height(720)
    , m_scanComplete(false)
{
    g_mainWindow = this;
    ZeroMemory(&m_wc, sizeof(m_wc));
}

MainWindow::~MainWindow() {
    shutdown();
    g_mainWindow = nullptr;
}

bool MainWindow::initialize(int width, int height) {
    m_width = width;
    m_height = height;
    
    // Регистрируем класс окна
    m_wc.cbSize = sizeof(WNDCLASSEXW);
    m_wc.style = CS_CLASSDC;
    m_wc.lpfnWndProc = WndProc;
    m_wc.cbClsExtra = 0;
    m_wc.cbWndExtra = 0;
    m_wc.hInstance = GetModuleHandle(nullptr);
    m_wc.hIcon = nullptr;
    m_wc.hCursor = nullptr;
    m_wc.hbrBackground = nullptr;
    m_wc.lpszMenuName = nullptr;
    m_wc.lpszClassName = L"ForensicScannerClass";
    m_wc.hIconSm = nullptr;
    
    RegisterClassExW(&m_wc);
    
    // Создаём окно
    m_hwnd = CreateWindowW(
        m_wc.lpszClassName,
        L"Форензик-сканер v1.0",
        WS_OVERLAPPEDWINDOW,
        100, 100, width, height,
        nullptr, nullptr, m_wc.hInstance, nullptr
    );
    
    if (!m_hwnd) {
        return false;
    }
    
    // Инициализируем DirectX
    if (!createDeviceD3D()) {
        cleanupDeviceD3D();
        UnregisterClassW(m_wc.lpszClassName, m_wc.hInstance);
        return false;
    }
    
    ShowWindow(m_hwnd, SW_SHOWDEFAULT);
    UpdateWindow(m_hwnd);
    
    // Инициализируем ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    // Загружаем шрифт с поддержкой кириллицы
    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesCyrillic());
    
    // Настраиваем стиль
    setupImGuiStyle();
    
    // Инициализируем бэкенды ImGui
    ImGui_ImplWin32_Init(m_hwnd);
    ImGui_ImplDX11_Init(m_pd3dDevice, m_pd3dDeviceContext);
    
    // Инициализируем логгер
    Logger::getInstance().initialize(L"scan_log.txt");
    
    // Загружаем конфигурацию
    m_config.loadConfig(L"config");
    
    // Настраиваем сканеры
    const ConfigData& configData = m_config.getData();
    
    m_hashScanner.loadHashDatabase(configData.hashDatabasePath);
    
    m_browserScanner.setKeywords(configData.keywords);
    m_browserScanner.setBlacklistSites(configData.blacklistSites);
    m_browserScanner.setKeywordWeight(configData.keywordMatchWeight);
    m_browserScanner.setSiteVisitWeight(configData.siteVisitWeight);
    
    m_discordScanner.setKeywords(configData.keywords);
    m_discordScanner.setBlacklist(configData.discordBlacklist);
    m_discordScanner.setDiscordPath(configData.discordPath);
    m_discordScanner.setRiskWeight(configData.discordMatchWeight);
    
    return true;
}

int MainWindow::run() {
    MSG msg;
    ZeroMemory(&msg, sizeof(msg));
    
    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }
        
        // Начинаем новый кадр ImGui
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        
        // Отрисовываем UI
        renderUI();
        
        // Рендерим
        ImGui::Render();
        const float clear_color[4] = { 0.1f, 0.1f, 0.12f, 1.0f };
        m_pd3dDeviceContext->OMSetRenderTargets(1, &m_mainRenderTargetView, nullptr);
        m_pd3dDeviceContext->ClearRenderTargetView(m_mainRenderTargetView, clear_color);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        
        m_pSwapChain->Present(1, 0); // VSync
    }
    
    return static_cast<int>(msg.wParam);
}

void MainWindow::shutdown() {
    // Ждём завершения сканирования
    if (m_scanThread.joinable()) {
        m_scanProgress.isRunning = false;
        m_scanThread.join();
    }
    
    // Очищаем ImGui
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    
    // Очищаем DirectX
    cleanupDeviceD3D();
    
    // Уничтожаем окно
    if (m_hwnd) {
        DestroyWindow(m_hwnd);
        m_hwnd = nullptr;
    }
    
    UnregisterClassW(m_wc.lpszClassName, m_wc.hInstance);
    
    // Закрываем логгер
    Logger::getInstance().shutdown();
}

bool MainWindow::createDeviceD3D() {
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = m_hwnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    
    UINT createDeviceFlags = 0;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };
    
    HRESULT res = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
        createDeviceFlags, featureLevelArray, 2,
        D3D11_SDK_VERSION, &sd,
        &m_pSwapChain, &m_pd3dDevice,
        &featureLevel, &m_pd3dDeviceContext
    );
    
    if (res != S_OK) {
        return false;
    }
    
    createRenderTarget();
    return true;
}

void MainWindow::cleanupDeviceD3D() {
    cleanupRenderTarget();
    if (m_pSwapChain) { m_pSwapChain->Release(); m_pSwapChain = nullptr; }
    if (m_pd3dDeviceContext) { m_pd3dDeviceContext->Release(); m_pd3dDeviceContext = nullptr; }
    if (m_pd3dDevice) { m_pd3dDevice->Release(); m_pd3dDevice = nullptr; }
}

void MainWindow::createRenderTarget() {
    ID3D11Texture2D* pBackBuffer;
    m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    m_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &m_mainRenderTargetView);
    pBackBuffer->Release();
}

void MainWindow::cleanupRenderTarget() {
    if (m_mainRenderTargetView) { 
        m_mainRenderTargetView->Release(); 
        m_mainRenderTargetView = nullptr; 
    }
}

LRESULT WINAPI MainWindow::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;
    
    switch (msg) {
        case WM_SIZE:
            if (g_mainWindow && g_mainWindow->m_pd3dDevice != nullptr && wParam != SIZE_MINIMIZED) {
                g_mainWindow->cleanupRenderTarget();
                g_mainWindow->m_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), 
                                                          DXGI_FORMAT_UNKNOWN, 0);
                g_mainWindow->createRenderTarget();
            }
            return 0;
            
        case WM_SYSCOMMAND:
            if ((wParam & 0xfff0) == SC_KEYMENU)
                return 0;
            break;
            
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

void MainWindow::renderUI() {
    // Полноэкранное окно ImGui
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                                    ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
                                    ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    
    ImGui::Begin("MainWindow", nullptr, window_flags);
    
    renderMainScreen();
    
    ImGui::End();
}

void MainWindow::renderMainScreen() {
    // Заголовок
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
    ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize(u8"ФОРЕНЗИК-СКАНЕР").x) * 0.5f);
    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), u8"ФОРЕНЗИК-СКАНЕР");
    ImGui::PopFont();
    
    ImGui::Separator();
    ImGui::Spacing();
    
    // Кнопка запуска сканирования
    ImGui::SetCursorPosX((ImGui::GetWindowWidth() - 200) * 0.5f);
    
    if (m_scanProgress.isRunning) {
        ImGui::BeginDisabled();
    }
    
    if (ImGui::Button(u8"ЗАПУСТИТЬ СКАНИРОВАНИЕ", ImVec2(200, 40))) {
        startScan();
    }
    
    if (m_scanProgress.isRunning) {
        ImGui::EndDisabled();
    }
    
    ImGui::Spacing();
    
    // Прогресс сканирования
    if (m_scanProgress.isRunning) {
        ImGui::Text(u8"Сканирование...");
        ImGui::ProgressBar(m_scanProgress.progress / 100.0f, ImVec2(-1, 20));
        
        std::wstring task = getCurrentTask();
        std::string taskUtf8 = wstringToUtf8(task);
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "%s", taskUtf8.c_str());
    }
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    // Индикатор риска и сводка
    if (m_scanComplete) {
        // Две колонки: индикатор риска слева, сводка справа
        ImGui::Columns(2, "ResultColumns", true);
        
        renderRiskIndicator();
        
        ImGui::NextColumn();
        
        renderSummary();
        
        ImGui::Columns(1);
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        // Вкладки логов
        renderLogTabs();
    } else if (!m_scanProgress.isRunning) {
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), 
                           u8"Нажмите кнопку для запуска сканирования системы.");
    }
}

void MainWindow::renderRiskIndicator() {
    ImGui::Text(u8"ОЦЕНКА РИСКА");
    ImGui::Spacing();
    
    // Получаем цвет для уровня риска
    float r, g, b, a;
    RiskCalculator::getRiskColor(m_riskSummary.riskLevel, r, g, b, a);
    
    // Большой процент
    char percentStr[16];
    sprintf(percentStr, "%d%%", m_riskSummary.totalRiskPercent);
    
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(r, g, b, a));
    ImGui::SetWindowFontScale(3.0f);
    ImGui::Text("%s", percentStr);
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopStyleColor();
    
    // Прогресс-бар с цветом
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(r, g, b, a));
    ImGui::ProgressBar(m_riskSummary.totalRiskPercent / 100.0f, ImVec2(200, 30));
    ImGui::PopStyleColor();
    
    // Уровень риска текстом
    std::wstring levelStr = RiskCalculator::riskLevelToString(m_riskSummary.riskLevel);
    ImGui::TextColored(ImVec4(r, g, b, a), u8"Уровень: %s", wstringToUtf8(levelStr).c_str());
    
    ImGui::Spacing();
    
    // Рекомендация
    std::string recUtf8 = wstringToUtf8(m_riskSummary.recommendation);
    ImGui::TextWrapped("%s", recUtf8.c_str());
}

void MainWindow::renderSummary() {
    ImGui::Text(u8"СВОДКА");
    ImGui::Spacing();
    
    // Таблица статистики
    if (ImGui::BeginTable("SummaryTable", 2, ImGuiTableFlags_Borders)) {
        ImGui::TableSetupColumn(u8"Категория", ImGuiTableColumnFlags_WidthFixed, 200);
        ImGui::TableSetupColumn(u8"Значение");
        ImGui::TableHeadersRow();
        
        // Хеши
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.6f, 1.0f), u8"Триггеры хешей");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%d (вклад: %d%%)", m_riskSummary.hashTriggers, m_riskSummary.hashRiskContribution);
        
        // Браузер
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), u8"Триггеры браузера");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%d (вклад: %d%%)", m_riskSummary.browserTriggers, m_riskSummary.browserRiskContribution);
        
        // Discord
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 1.0f, 1.0f), u8"Триггеры Discord");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%d (вклад: %d%%)", m_riskSummary.discordTriggers, m_riskSummary.discordRiskContribution);
        
        // Итого
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.6f, 1.0f), u8"ИТОГО");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%d%%", m_riskSummary.totalRiskPercent);
        
        ImGui::EndTable();
    }
}

void MainWindow::renderLogTabs() {
    if (ImGui::BeginTabBar("LogTabs")) {
        if (ImGui::BeginTabItem(u8"Лог хешей")) {
            m_currentTab = 0;
            renderHashLog();
            ImGui::EndTabItem();
        }
        
        if (ImGui::BeginTabItem(u8"Лог браузера")) {
            m_currentTab = 1;
            renderBrowserLog();
            ImGui::EndTabItem();
        }
        
        if (ImGui::BeginTabItem(u8"Лог Discord")) {
            m_currentTab = 2;
            renderDiscordLog();
            ImGui::EndTabItem();
        }
        
        ImGui::EndTabBar();
    }
}

void MainWindow::renderHashLog() {
    auto entries = Logger::getInstance().getEntriesByCategory(LogCategory::HASH);
    
    if (entries.empty()) {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), u8"Нет записей в логе хешей.");
        return;
    }
    
    ImGui::BeginChild("HashLogScroll", ImVec2(0, 300), true);
    
    int index = 0;
    for (auto& entry : entries) {
        renderLogEntry(entry, index++);
    }
    
    ImGui::EndChild();
}

void MainWindow::renderBrowserLog() {
    auto entries = Logger::getInstance().getEntriesByCategory(LogCategory::BROWSER);
    
    if (entries.empty()) {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), u8"Нет записей в логе браузера.");
        return;
    }
    
    ImGui::BeginChild("BrowserLogScroll", ImVec2(0, 300), true);
    
    int index = 0;
    for (auto& entry : entries) {
        renderLogEntry(entry, index++);
    }
    
    ImGui::EndChild();
}

void MainWindow::renderDiscordLog() {
    auto entries = Logger::getInstance().getEntriesByCategory(LogCategory::DISCORD);
    
    if (entries.empty()) {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), u8"Нет записей в логе Discord.");
        return;
    }
    
    ImGui::BeginChild("DiscordLogScroll", ImVec2(0, 300), true);
    
    int index = 0;
    for (auto& entry : entries) {
        renderLogEntry(entry, index++);
    }
    
    ImGui::EndChild();
}

void MainWindow::renderLogEntry(LogEntry& entry, int index) {
    std::string headerLabel = wstringToUtf8(entry.matchedValue);
    
    // Цвет в зависимости от риска
    float intensity = entry.riskWeight / 50.0f;
    ImVec4 color(0.8f + intensity * 0.2f, 0.4f, 0.4f, 1.0f);
    
    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f, 0.2f, 0.25f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.3f, 0.3f, 0.35f, 1.0f));
    
    char nodeId[64];
    sprintf(nodeId, "%s##%d", headerLabel.c_str(), index);
    
    if (ImGui::CollapsingHeader(nodeId)) {
        ImGui::Indent();
        
        // Тип триггера
        std::wstring triggerTypeStr = Logger::triggerTypeToString(entry.triggerType);
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), u8"Тип триггера:");
        ImGui::SameLine();
        ImGui::Text("%s", wstringToUtf8(triggerTypeStr).c_str());
        
        // Источник
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), u8"Источник:");
        ImGui::SameLine();
        ImGui::TextWrapped("%s", wstringToUtf8(entry.source).c_str());
        
        // Совпавшее значение
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), u8"Значение:");
        ImGui::SameLine();
        ImGui::TextColored(color, "%s", headerLabel.c_str());
        
        // Детали
        if (!entry.details.empty()) {
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), u8"Детали:");
            ImGui::SameLine();
            ImGui::TextWrapped("%s", wstringToUtf8(entry.details).c_str());
        }
        
        // Вес риска
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), u8"Вес риска:");
        ImGui::SameLine();
        ImGui::Text("%d%%", entry.riskWeight);
        
        // Время
        std::tm* tm = std::localtime(&entry.timestamp);
        char timeStr[64];
        sprintf(timeStr, "%02d.%02d.%04d %02d:%02d:%02d",
                tm->tm_mday, tm->tm_mon + 1, tm->tm_year + 1900,
                tm->tm_hour, tm->tm_min, tm->tm_sec);
        
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), u8"Время:");
        ImGui::SameLine();
        ImGui::Text("%s", timeStr);
        
        ImGui::Unindent();
    }
    
    ImGui::PopStyleColor(2);
}

void MainWindow::startScan() {
    if (m_scanProgress.isRunning) {
        return;
    }
    
    // Очищаем предыдущие результаты
    Logger::getInstance().clear();
    m_hashScanner.clearResults();
    m_browserScanner.clearResults();
    m_discordScanner.clearResults();
    m_riskCalculator.clear();
    m_scanComplete = false;
    
    // Запускаем сканирование в отдельном потоке
    m_scanProgress.isRunning = true;
    m_scanProgress.progress = 0;
    
    m_scanThread = std::thread(&MainWindow::scanProcedure, this);
    m_scanThread.detach();
}

void MainWindow::scanProcedure() {
    const ConfigData& config = m_config.getData();
    
    try {
        // Фаза 1: Сканирование хешей (0-40%)
        updateTask(L"Сканирование файлов на хеши...");
        
        for (const auto& dir : config.scanDirectories) {
            std::wstring expandedDir = Config::expandEnvironmentPath(dir);
            updateTask(L"Сканирование: " + expandedDir);
            
            m_hashScanner.scanDirectory(expandedDir, true, 
                [this](int current, int total, const std::wstring& file) {
                    int baseProgress = 0;
                    int phaseProgress = (current * 40) / std::max(1, total);
                    m_scanProgress.progress = baseProgress + phaseProgress;
                });
        }
        
        m_scanProgress.progress = 40;
        
        // Фаза 2: Сканирование браузера (40-70%)
        updateTask(L"Анализ истории браузеров...");
        
        m_browserScanner.scanAllBrowsers([this](const std::wstring& status, int progress) {
            updateTask(status);
            m_scanProgress.progress = 40 + (progress * 30) / 100;
        });
        
        m_scanProgress.progress = 70;
        
        // Фаза 3: Сканирование Discord (70-95%)
        updateTask(L"Анализ данных Discord...");
        
        m_discordScanner.scan([this](const std::wstring& status, int progress) {
            updateTask(status);
            m_scanProgress.progress = 70 + (progress * 25) / 100;
        });
        
        m_scanProgress.progress = 95;
        
        // Фаза 4: Расчёт риска (95-100%)
        updateTask(L"Расчёт оценки риска...");
        
        m_riskCalculator.setHashResults(m_hashScanner.getMatches());
        m_riskCalculator.setBrowserResults(m_browserScanner.getMatches());
        m_riskCalculator.setDiscordResults(m_discordScanner.getMatches());
        
        m_riskSummary = m_riskCalculator.calculate();
        
        m_scanProgress.progress = 100;
        m_scanComplete = true;
        
        Logger::getInstance().info(LogCategory::SYSTEM, 
            L"Сканирование завершено. Риск: " + std::to_wstring(m_riskSummary.totalRiskPercent) + L"%");
        
    } catch (const std::exception& e) {
        Logger::getInstance().error(LogCategory::SYSTEM, L"Ошибка сканирования");
    }
    
    m_scanProgress.isRunning = false;
}

void MainWindow::updateTask(const std::wstring& task) {
    std::lock_guard<std::mutex> lock(m_scanProgress.taskMutex);
    m_scanProgress.currentTask = task;
}

std::wstring MainWindow::getCurrentTask() {
    std::lock_guard<std::mutex> lock(m_scanProgress.taskMutex);
    return m_scanProgress.currentTask;
}

void MainWindow::setupImGuiStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    
    // Тёмная тема
    style.WindowRounding = 5.0f;
    style.FrameRounding = 3.0f;
    style.ScrollbarRounding = 3.0f;
    style.GrabRounding = 3.0f;
    style.TabRounding = 3.0f;
    
    style.WindowPadding = ImVec2(15, 15);
    style.FramePadding = ImVec2(8, 5);
    style.ItemSpacing = ImVec2(10, 8);
    style.ItemInnerSpacing = ImVec2(8, 6);
    
    ImVec4* colors = style.Colors;
    
    colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.12f, 1.0f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.15f, 0.15f, 0.17f, 1.0f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.12f, 0.12f, 0.14f, 1.0f);
    colors[ImGuiCol_Border] = ImVec4(0.3f, 0.3f, 0.35f, 0.5f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.2f, 0.2f, 0.22f, 1.0f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.25f, 0.25f, 0.28f, 1.0f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.3f, 0.3f, 0.33f, 1.0f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.15f, 0.15f, 0.17f, 1.0f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.2f, 0.2f, 0.22f, 1.0f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.15f, 0.15f, 0.17f, 1.0f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.15f, 0.15f, 0.17f, 1.0f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.3f, 0.3f, 0.35f, 1.0f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.4f, 0.4f, 0.45f, 1.0f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.5f, 0.5f, 0.55f, 1.0f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.4f, 0.8f, 0.4f, 1.0f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.4f, 0.7f, 1.0f, 1.0f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.5f, 0.8f, 1.0f, 1.0f);
    colors[ImGuiCol_Button] = ImVec4(0.25f, 0.5f, 0.8f, 1.0f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.35f, 0.6f, 0.9f, 1.0f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.2f, 0.45f, 0.75f, 1.0f);
    colors[ImGuiCol_Header] = ImVec4(0.25f, 0.25f, 0.28f, 1.0f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.35f, 0.35f, 0.38f, 1.0f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.4f, 0.4f, 0.43f, 1.0f);
    colors[ImGuiCol_Separator] = ImVec4(0.3f, 0.3f, 0.35f, 0.5f);
    colors[ImGuiCol_Tab] = ImVec4(0.2f, 0.2f, 0.22f, 1.0f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.35f, 0.55f, 0.8f, 1.0f);
    colors[ImGuiCol_TabActive] = ImVec4(0.3f, 0.5f, 0.75f, 1.0f);
    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.2f, 0.2f, 0.22f, 1.0f);
    colors[ImGuiCol_TableBorderStrong] = ImVec4(0.3f, 0.3f, 0.35f, 1.0f);
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.25f, 0.25f, 0.28f, 1.0f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.4f, 0.7f, 1.0f, 1.0f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.3f, 0.5f, 0.8f, 0.5f);
}

std::string MainWindow::wstringToUtf8(const std::wstring& wstr) {
    if (wstr.empty()) return std::string();
    
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), 
                                          nullptr, 0, nullptr, nullptr);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), 
                        &strTo[0], size_needed, nullptr, nullptr);
    return strTo;
}

std::wstring MainWindow::utf8ToWstring(const std::string& str) {
    if (str.empty()) return std::wstring();
    
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), 
                                          nullptr, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), 
                        &wstrTo[0], size_needed);
    return wstrTo;
}

} // namespace ForensicScanner
