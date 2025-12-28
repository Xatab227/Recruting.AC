#include "MainWindow.h"
#include "../Config.h"
#include "../Logger.h"
#include "../Utils.h"
#include "../HashScanner.h"
#include "../BrowserScanner.h"
#include "../DiscordScanner.h"
#include "../RiskCalculator.h"

#ifdef _WIN32
#include <windows.h>
#include <d3d11.h>
#include <tchar.h>
#include <dwmapi.h>

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace ForensicScanner {

// Window procedure
static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg) {
        case WM_SIZE:
            if (wParam != SIZE_MINIMIZED) {
                // Resize handling is done in the render loop
            }
            return 0;
        case WM_SYSCOMMAND:
            if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
                return 0;
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

MainWindow::MainWindow() {
}

MainWindow::~MainWindow() {
    Shutdown();
}

bool MainWindow::Initialize(const std::string& title, int width, int height) {
    m_title = title;
    m_width = width;
    m_height = height;
    
    // Create application window
    WNDCLASSEX wc = {
        sizeof(WNDCLASSEX),
        CS_CLASSDC,
        WndProc,
        0L, 0L,
        GetModuleHandle(nullptr),
        nullptr, nullptr, nullptr, nullptr,
        _T("ForensicScannerClass"),
        nullptr
    };
    RegisterClassEx(&wc);
    
    HWND hwnd = CreateWindow(
        wc.lpszClassName,
        Utils::StringToWString(title).c_str(),
        WS_OVERLAPPEDWINDOW,
        100, 100, width, height,
        nullptr, nullptr,
        wc.hInstance,
        nullptr
    );
    
    m_hwnd = hwnd;
    
    // Initialize Direct3D
    if (!CreateDeviceD3D()) {
        CleanupDeviceD3D();
        UnregisterClass(wc.lpszClassName, wc.hInstance);
        return false;
    }
    
    // Show the window
    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);
    
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    // Настройка шрифтов с поддержкой кириллицы
    ImFontConfig fontConfig;
    fontConfig.OversampleH = 2;
    fontConfig.OversampleV = 1;
    
    // Используем встроенный шрифт с расширенным набором глифов
    io.Fonts->AddFontDefault();
    
    // Попытка загрузить системный шрифт с кириллицей
    const char* fontPaths[] = {
        "C:\\Windows\\Fonts\\segoeui.ttf",
        "C:\\Windows\\Fonts\\arial.ttf",
        "C:\\Windows\\Fonts\\tahoma.ttf"
    };
    
    for (const char* fontPath : fontPaths) {
        if (Utils::FileExists(fontPath)) {
            static const ImWchar ranges[] = {
                0x0020, 0x00FF, // Basic Latin + Latin Supplement
                0x0400, 0x052F, // Cyrillic + Cyrillic Supplement
                0x2DE0, 0x2DFF, // Cyrillic Extended-A
                0xA640, 0xA69F, // Cyrillic Extended-B
                0,
            };
            io.Fonts->AddFontFromFileTTF(fontPath, 16.0f, &fontConfig, ranges);
            break;
        }
    }
    
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    
    // Customize style
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 5.0f;
    style.FrameRounding = 3.0f;
    style.ScrollbarRounding = 3.0f;
    style.GrabRounding = 3.0f;
    style.WindowPadding = ImVec2(10, 10);
    style.FramePadding = ImVec2(8, 4);
    style.ItemSpacing = ImVec2(8, 6);
    
    // Colors
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.12f, 1.0f);
    colors[ImGuiCol_Header] = ImVec4(0.2f, 0.2f, 0.25f, 1.0f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.3f, 0.3f, 0.35f, 1.0f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.35f, 0.35f, 0.4f, 1.0f);
    colors[ImGuiCol_Button] = ImVec4(0.2f, 0.4f, 0.6f, 1.0f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.3f, 0.5f, 0.7f, 1.0f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.25f, 0.45f, 0.65f, 1.0f);
    colors[ImGuiCol_Tab] = ImVec4(0.15f, 0.15f, 0.18f, 1.0f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.35f, 0.35f, 0.4f, 1.0f);
    colors[ImGuiCol_TabActive] = ImVec4(0.25f, 0.25f, 0.3f, 1.0f);
    
    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(
        reinterpret_cast<ID3D11Device*>(m_d3dDevice),
        reinterpret_cast<ID3D11DeviceContext*>(m_d3dContext)
    );
    
    m_initialized = true;
    
    // Инициализация логгера
    Logger::Instance().Initialize("scan_log.txt");
    Logger::Instance().Info("Программа запущена", "system");
    
    // Загрузка конфигурации
    if (!Config::Instance().LoadAll("config")) {
        Logger::Instance().Warning("Не удалось загрузить все конфигурационные файлы", "system");
    }
    
    return true;
}

void MainWindow::Run() {
    MSG msg;
    ZeroMemory(&msg, sizeof(msg));
    
    while (!m_shouldClose) {
        while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT) {
                m_shouldClose = true;
            }
        }
        
        if (m_shouldClose)
            break;
        
        RenderFrame();
    }
}

void MainWindow::Shutdown() {
    if (!m_initialized)
        return;
    
    // Останавливаем сканирование
    if (m_scanning) {
        StopScan();
        if (m_scanThread.joinable()) {
            m_scanThread.join();
        }
    }
    
    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    
    CleanupDeviceD3D();
    DestroyWindow(reinterpret_cast<HWND>(m_hwnd));
    UnregisterClass(_T("ForensicScannerClass"), GetModuleHandle(nullptr));
    
    Logger::Instance().Info("Программа завершена", "system");
    Logger::Instance().Shutdown();
    
    m_initialized = false;
}

bool MainWindow::CreateDeviceD3D() {
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
    sd.OutputWindow = reinterpret_cast<HWND>(m_hwnd);
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    
    UINT createDeviceFlags = 0;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_0
    };
    
    ID3D11Device* device;
    ID3D11DeviceContext* context;
    IDXGISwapChain* swapChain;
    
    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        createDeviceFlags,
        featureLevelArray,
        2,
        D3D11_SDK_VERSION,
        &sd,
        &swapChain,
        &device,
        &featureLevel,
        &context
    );
    
    if (FAILED(hr))
        return false;
    
    m_d3dDevice = device;
    m_d3dContext = context;
    m_swapChain = swapChain;
    
    CreateRenderTarget();
    return true;
}

void MainWindow::CleanupDeviceD3D() {
    CleanupRenderTarget();
    if (m_swapChain) {
        reinterpret_cast<IDXGISwapChain*>(m_swapChain)->Release();
        m_swapChain = nullptr;
    }
    if (m_d3dContext) {
        reinterpret_cast<ID3D11DeviceContext*>(m_d3dContext)->Release();
        m_d3dContext = nullptr;
    }
    if (m_d3dDevice) {
        reinterpret_cast<ID3D11Device*>(m_d3dDevice)->Release();
        m_d3dDevice = nullptr;
    }
}

void MainWindow::CreateRenderTarget() {
    ID3D11Texture2D* pBackBuffer;
    reinterpret_cast<IDXGISwapChain*>(m_swapChain)->GetBuffer(
        0, IID_PPV_ARGS(&pBackBuffer));
    
    ID3D11RenderTargetView* rtv;
    reinterpret_cast<ID3D11Device*>(m_d3dDevice)->CreateRenderTargetView(
        pBackBuffer, nullptr, &rtv);
    m_renderTargetView = rtv;
    pBackBuffer->Release();
}

void MainWindow::CleanupRenderTarget() {
    if (m_renderTargetView) {
        reinterpret_cast<ID3D11RenderTargetView*>(m_renderTargetView)->Release();
        m_renderTargetView = nullptr;
    }
}

void MainWindow::RenderFrame() {
    // Start the Dear ImGui frame
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    
    // Render UI
    RenderUI();
    
    // Rendering
    ImGui::Render();
    const float clear_color[4] = {0.1f, 0.1f, 0.12f, 1.0f};
    auto* rtv = reinterpret_cast<ID3D11RenderTargetView*>(m_renderTargetView);
    reinterpret_cast<ID3D11DeviceContext*>(m_d3dContext)->OMSetRenderTargets(1, &rtv, nullptr);
    reinterpret_cast<ID3D11DeviceContext*>(m_d3dContext)->ClearRenderTargetView(
        rtv, clear_color);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    
    reinterpret_cast<IDXGISwapChain*>(m_swapChain)->Present(1, 0);
}

void MainWindow::RenderUI() {
    // Создаём главное окно на весь экран
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    
    ImGuiWindowFlags flags = 
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoBringToFrontOnFocus;
    
    ImGui::Begin("MainWindow", nullptr, flags);
    
    RenderMainPanel();
    
    ImGui::End();
}

void MainWindow::RenderMainPanel() {
    // Заголовок
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
    ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f), "FORENSIC SCANNER");
    ImGui::PopFont();
    ImGui::Separator();
    ImGui::Spacing();
    
    // Левая панель - индикатор риска и кнопки
    ImGui::BeginChild("LeftPanel", ImVec2(300, 0), true);
    
    RenderRiskIndicator();
    ImGui::Spacing();
    RenderControlButtons();
    ImGui::Spacing();
    RenderProgressBar();
    ImGui::Spacing();
    RenderSummary();
    
    ImGui::EndChild();
    
    ImGui::SameLine();
    
    // Правая панель - логи
    ImGui::BeginChild("RightPanel", ImVec2(0, 0), true);
    
    RenderLogTabs();
    
    ImGui::EndChild();
}

void MainWindow::RenderRiskIndicator() {
    ImGui::Text("Оценка риска:");
    ImGui::Spacing();
    
    // Получаем цвет на основе уровня риска
    float r, g, b, a;
    RiskCalculator::GetRiskColorByPercent(m_results.totalRiskPercent, r, g, b, a);
    
    // Большой индикатор процента
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(r, g, b, a));
    
    char percentText[16];
    snprintf(percentText, sizeof(percentText), "%d%%", m_results.totalRiskPercent);
    
    ImVec2 textSize = ImGui::CalcTextSize(percentText);
    float centerX = (ImGui::GetContentRegionAvail().x - textSize.x * 2) / 2;
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + centerX);
    
    ImGui::SetWindowFontScale(2.0f);
    ImGui::Text("%s", percentText);
    ImGui::SetWindowFontScale(1.0f);
    
    ImGui::PopStyleColor();
    
    // Прогресс-бар риска
    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(r, g, b, a));
    ImGui::ProgressBar(m_results.totalRiskPercent / 100.0f, ImVec2(-1, 20), "");
    ImGui::PopStyleColor();
    
    // Текстовое описание уровня
    ImGui::Spacing();
    ImGui::TextWrapped("%s", RiskCalculator::GetRiskDescriptionByPercent(
        m_results.totalRiskPercent).c_str());
}

void MainWindow::RenderControlButtons() {
    ImGui::Separator();
    ImGui::Text("Управление:");
    ImGui::Spacing();
    
    if (m_scanning) {
        if (ImGui::Button("Остановить", ImVec2(-1, 35))) {
            StopScan();
        }
    } else {
        if (ImGui::Button("Начать сканирование", ImVec2(-1, 35))) {
            StartScan();
        }
    }
    
    if (ImGui::Button("Экспорт отчёта", ImVec2(-1, 30))) {
        ExportReport();
    }
    
    if (ImGui::Button("Очистить результаты", ImVec2(-1, 30))) {
        ClearResults();
    }
}

void MainWindow::RenderProgressBar() {
    if (m_scanning) {
        ImGui::Separator();
        ImGui::Text("Прогресс сканирования:");
        ImGui::ProgressBar(m_scanProgress, ImVec2(-1, 20));
        
        std::lock_guard<std::mutex> lock(m_statusMutex);
        ImGui::TextWrapped("%s", m_scanStatus.c_str());
    }
}

void MainWindow::RenderSummary() {
    ImGui::Separator();
    ImGui::Text("Сводка:");
    ImGui::Spacing();
    
    ImGui::BulletText("Триггеров по хешам: %d", m_results.hashMatches);
    ImGui::BulletText("Ключевых слов: %d", m_results.keywordsFound);
    ImGui::BulletText("Подозрительных URL: %d", m_results.suspiciousURLs);
    ImGui::BulletText("Триггеров Discord: %d", m_results.discordMatches);
    ImGui::Spacing();
    ImGui::BulletText("Файлов просканировано: %d", m_results.filesScanned);
}

void MainWindow::RenderLogTabs() {
    if (ImGui::BeginTabBar("LogTabs")) {
        if (ImGui::BeginTabItem("Лог хешей")) {
            m_currentTab = 0;
            RenderHashLog();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Лог браузера")) {
            m_currentTab = 1;
            RenderBrowserLog();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Лог Discord")) {
            m_currentTab = 2;
            RenderDiscordLog();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
}

void MainWindow::RenderHashLog() {
    std::lock_guard<std::mutex> lock(m_resultsMutex);
    
    if (m_results.hashTriggers.empty()) {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), 
            "Триггеров не обнаружено");
        return;
    }
    
    for (size_t i = 0; i < m_results.hashTriggers.size(); ++i) {
        RenderTriggerDetails(m_results.hashTriggers[i], static_cast<int>(i));
    }
}

void MainWindow::RenderBrowserLog() {
    std::lock_guard<std::mutex> lock(m_resultsMutex);
    
    if (m_results.browserTriggers.empty()) {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), 
            "Триггеров не обнаружено");
        return;
    }
    
    for (size_t i = 0; i < m_results.browserTriggers.size(); ++i) {
        RenderTriggerDetails(m_results.browserTriggers[i], 
            static_cast<int>(i + 1000));
    }
}

void MainWindow::RenderDiscordLog() {
    std::lock_guard<std::mutex> lock(m_resultsMutex);
    
    if (m_results.discordTriggers.empty()) {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), 
            "Триггеров не обнаружено");
        return;
    }
    
    for (size_t i = 0; i < m_results.discordTriggers.size(); ++i) {
        RenderTriggerDetails(m_results.discordTriggers[i], 
            static_cast<int>(i + 2000));
    }
}

void MainWindow::RenderTriggerDetails(const TriggerEvent& trigger, int index) {
    ImGui::PushID(index);
    
    // Цвет в зависимости от веса
    float r, g, b, a;
    if (trigger.weight >= 30) {
        r = 0.9f; g = 0.3f; b = 0.3f; a = 1.0f;
    } else if (trigger.weight >= 15) {
        r = 1.0f; g = 0.7f; b = 0.0f; a = 1.0f;
    } else {
        r = 0.3f; g = 0.8f; b = 0.3f; a = 1.0f;
    }
    
    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(r * 0.3f, g * 0.3f, b * 0.3f, 0.5f));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(r * 0.4f, g * 0.4f, b * 0.4f, 0.6f));
    
    // Заголовок элемента
    char header[256];
    snprintf(header, sizeof(header), "[%d%%] %s: %s", 
        trigger.weight,
        TriggerTypeToString(trigger.type).c_str(),
        trigger.matchedValue.c_str());
    
    bool open = ImGui::CollapsingHeader(header);
    
    ImGui::PopStyleColor(2);
    
    if (open) {
        ImGui::Indent(20);
        
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.8f, 1.0f), "Тип:");
        ImGui::SameLine();
        ImGui::Text("%s", TriggerTypeToString(trigger.type).c_str());
        
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.8f, 1.0f), "Источник:");
        ImGui::SameLine();
        ImGui::TextWrapped("%s", trigger.source.c_str());
        
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.8f, 1.0f), "Значение:");
        ImGui::SameLine();
        ImGui::TextWrapped("%s", trigger.matchedValue.c_str());
        
        if (!trigger.details.empty()) {
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.8f, 1.0f), "Детали:");
            ImGui::SameLine();
            ImGui::TextWrapped("%s", trigger.details.c_str());
        }
        
        if (!trigger.context.empty()) {
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.8f, 1.0f), "Контекст:");
            ImGui::SameLine();
            ImGui::TextWrapped("%s", trigger.context.c_str());
        }
        
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.8f, 1.0f), "Вес триггера:");
        ImGui::SameLine();
        ImGui::Text("%d%%", trigger.weight);
        
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.8f, 1.0f), "Время:");
        ImGui::SameLine();
        ImGui::Text("%s", Utils::FormatTimestamp(trigger.timestamp).c_str());
        
        ImGui::Unindent(20);
        ImGui::Spacing();
    }
    
    ImGui::PopID();
}

void MainWindow::RenderStatusBar() {
    // Статус-бар внизу окна
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, 
        viewport->Pos.y + viewport->Size.y - 25));
    ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, 25));
    
    ImGuiWindowFlags flags = 
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar;
    
    ImGui::Begin("StatusBar", nullptr, flags);
    
    if (m_scanning) {
        ImGui::Text("Сканирование...");
    } else if (m_results.scanComplete) {
        ImGui::Text("Сканирование завершено");
    } else {
        ImGui::Text("Готов к сканированию");
    }
    
    ImGui::End();
}

void MainWindow::StartScan() {
    if (m_scanning) return;
    
    ClearResults();
    m_scanning = true;
    m_stopScan = false;
    m_scanProgress = 0.0f;
    
    m_scanThread = std::thread(&MainWindow::ScanThread, this);
}

void MainWindow::StopScan() {
    m_stopScan = true;
    if (m_scanThread.joinable()) {
        m_scanThread.join();
    }
    m_scanning = false;
}

void MainWindow::ScanThread() {
    Logger::Instance().Info("Начало сканирования системы...", "scan");
    
    ScanResults results;
    results.scanComplete = false;
    
    // Сканирование хешей
    {
        std::lock_guard<std::mutex> lock(m_statusMutex);
        m_scanStatus = "Сканирование хешей файлов...";
    }
    
    HashScanner hashScanner;
    hashScanner.SetProgressCallback([this](float progress, const std::string& file) {
        m_scanProgress = progress * 0.33f;
        std::lock_guard<std::mutex> lock(m_statusMutex);
        m_scanStatus = "Хеши: " + file;
    });
    
    if (!m_stopScan) {
        hashScanner.Scan(results.hashTriggers);
        results.filesScanned = hashScanner.GetScannedCount();
    }
    
    // Сканирование браузера
    {
        std::lock_guard<std::mutex> lock(m_statusMutex);
        m_scanStatus = "Сканирование браузеров...";
    }
    
    BrowserScanner browserScanner;
    browserScanner.SetProgressCallback([this](float progress, const std::string& browser) {
        m_scanProgress = 0.33f + progress * 0.33f;
        std::lock_guard<std::mutex> lock(m_statusMutex);
        m_scanStatus = "Браузер: " + browser;
    });
    
    if (!m_stopScan) {
        browserScanner.Scan(results.browserTriggers);
    }
    
    // Сканирование Discord
    {
        std::lock_guard<std::mutex> lock(m_statusMutex);
        m_scanStatus = "Сканирование Discord...";
    }
    
    DiscordScanner discordScanner;
    discordScanner.SetProgressCallback([this](float progress, const std::string& op) {
        m_scanProgress = 0.66f + progress * 0.34f;
        std::lock_guard<std::mutex> lock(m_statusMutex);
        m_scanStatus = "Discord: " + op;
    });
    
    if (!m_stopScan) {
        discordScanner.Scan(results.discordTriggers);
    }
    
    // Расчёт риска
    {
        std::lock_guard<std::mutex> lock(m_statusMutex);
        m_scanStatus = "Расчёт риска...";
    }
    
    RiskCalculator calculator;
    calculator.Calculate(results);
    
    results.scanComplete = true;
    
    // Сохраняем результаты
    {
        std::lock_guard<std::mutex> lock(m_resultsMutex);
        m_results = results;
    }
    
    m_scanProgress = 1.0f;
    
    {
        std::lock_guard<std::mutex> lock(m_statusMutex);
        m_scanStatus = "Сканирование завершено";
    }
    
    Logger::Instance().Info("Сканирование системы завершено", "scan");
    
    m_scanning = false;
}

void MainWindow::ExportReport() {
    // Экспорт отчёта в текстовый файл
    std::string filename = "report_" + 
        std::to_string(Utils::GetCurrentTimestamp()) + ".txt";
    
    std::ofstream file(filename);
    if (!file.is_open()) {
        Logger::Instance().Error("Не удалось создать файл отчёта", "system");
        return;
    }
    
    file << "========================================\n";
    file << "  FORENSIC SCANNER - ОТЧЁТ\n";
    file << "  Дата: " << Utils::FormatTimestamp(Utils::GetCurrentTimestamp()) << "\n";
    file << "========================================\n\n";
    
    file << "ОБЩИЙ РИСК: " << m_results.totalRiskPercent << "%\n";
    file << "Уровень: " << RiskLevelToString(m_results.riskLevel) << "\n\n";
    
    file << "СВОДКА:\n";
    file << "  Триггеров по хешам: " << m_results.hashMatches << "\n";
    file << "  Ключевых слов: " << m_results.keywordsFound << "\n";
    file << "  Подозрительных URL: " << m_results.suspiciousURLs << "\n";
    file << "  Триггеров Discord: " << m_results.discordMatches << "\n";
    file << "  Файлов просканировано: " << m_results.filesScanned << "\n\n";
    
    auto writeTriggers = [&file](const std::vector<TriggerEvent>& triggers, 
                                  const std::string& category) {
        file << "--- " << category << " ---\n";
        if (triggers.empty()) {
            file << "  (нет триггеров)\n";
        }
        for (const auto& t : triggers) {
            file << "  [" << t.weight << "%] " << TriggerTypeToString(t.type) << "\n";
            file << "    Источник: " << t.source << "\n";
            file << "    Значение: " << t.matchedValue << "\n";
            if (!t.context.empty()) {
                file << "    Контекст: " << t.context << "\n";
            }
            file << "\n";
        }
    };
    
    writeTriggers(m_results.hashTriggers, "ТРИГГЕРЫ ХЕШЕЙ");
    writeTriggers(m_results.browserTriggers, "ТРИГГЕРЫ БРАУЗЕРА");
    writeTriggers(m_results.discordTriggers, "ТРИГГЕРЫ DISCORD");
    
    file << "========================================\n";
    file << "  Конец отчёта\n";
    file << "========================================\n";
    
    file.close();
    
    Logger::Instance().Info("Отчёт сохранён: " + filename, "system");
}

void MainWindow::ClearResults() {
    std::lock_guard<std::mutex> lock(m_resultsMutex);
    m_results = ScanResults();
    Logger::Instance().ClearLogs();
}

void MainWindow::UpdateProgress(float progress, const std::string& message) {
    m_scanProgress = progress;
    std::lock_guard<std::mutex> lock(m_statusMutex);
    m_scanStatus = message;
}

} // namespace ForensicScanner

#else
// Заглушка для не-Windows систем

namespace ForensicScanner {

MainWindow::MainWindow() {}
MainWindow::~MainWindow() {}

bool MainWindow::Initialize(const std::string& title, int width, int height) {
    return false;
}

void MainWindow::Run() {}
void MainWindow::Shutdown() {}
bool MainWindow::CreateDeviceD3D() { return false; }
void MainWindow::CleanupDeviceD3D() {}
void MainWindow::CreateRenderTarget() {}
void MainWindow::CleanupRenderTarget() {}
void MainWindow::RenderFrame() {}
void MainWindow::RenderUI() {}
void MainWindow::RenderMainPanel() {}
void MainWindow::RenderRiskIndicator() {}
void MainWindow::RenderSummary() {}
void MainWindow::RenderLogTabs() {}
void MainWindow::RenderHashLog() {}
void MainWindow::RenderBrowserLog() {}
void MainWindow::RenderDiscordLog() {}
void MainWindow::RenderTriggerDetails(const TriggerEvent&, int) {}
void MainWindow::RenderControlButtons() {}
void MainWindow::RenderProgressBar() {}
void MainWindow::RenderStatusBar() {}
void MainWindow::StartScan() {}
void MainWindow::StopScan() {}
void MainWindow::ExportReport() {}
void MainWindow::ClearResults() {}
void MainWindow::ScanThread() {}
void MainWindow::UpdateProgress(float, const std::string&) {}

} // namespace ForensicScanner

#endif
