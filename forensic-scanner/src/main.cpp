/**
 * @file main.cpp
 * @brief Точка входа в программу форензик-сканера
 * 
 * Forensic Scanner - программа для анализа системы на наличие
 * признаков использования читов и запрещённых модификаций
 * 
 * @author Forensic Scanner Team
 * @version 1.0
 */

#include <Windows.h>
#include "../include/MainWindow.h"
#include "../include/Logger.h"

/**
 * @brief Точка входа Windows приложения
 */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
                   LPSTR lpCmdLine, int nCmdShow)
{
    // Инициализируем COM для корректной работы с системными API
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hr)) {
        MessageBoxW(nullptr, L"Не удалось инициализировать COM", L"Ошибка", MB_OK | MB_ICONERROR);
        return 1;
    }
    
    // Создаём и инициализируем главное окно
    ForensicScanner::MainWindow mainWindow;
    
    if (!mainWindow.initialize(1280, 720)) {
        MessageBoxW(nullptr, L"Не удалось инициализировать приложение", L"Ошибка", MB_OK | MB_ICONERROR);
        CoUninitialize();
        return 1;
    }
    
    // Запускаем главный цикл
    int result = mainWindow.run();
    
    // Очищаем ресурсы
    mainWindow.shutdown();
    CoUninitialize();
    
    return result;
}

/**
 * @brief Альтернативная консольная точка входа для отладки
 */
int main(int argc, char* argv[])
{
    return WinMain(GetModuleHandle(nullptr), nullptr, GetCommandLineA(), SW_SHOWNORMAL);
}
