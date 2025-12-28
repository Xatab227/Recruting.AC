#include "UI/MainWindow.h"
#include "Config.h"
#include "Logger.h"

#ifdef _WIN32
#include <windows.h>

int WINAPI WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine,
    _In_ int nShowCmd
) {
    // Устанавливаем кодировку для консоли (если используется)
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    
    // Создаём и инициализируем главное окно
    ForensicScanner::MainWindow window;
    
    if (!window.Initialize("Forensic Scanner v1.0", 1280, 720)) {
        MessageBoxW(nullptr, 
            L"Не удалось инициализировать приложение.\n"
            L"Убедитесь, что установлены все необходимые компоненты.",
            L"Ошибка",
            MB_OK | MB_ICONERROR);
        return 1;
    }
    
    // Запускаем главный цикл
    window.Run();
    
    // Завершаем работу
    window.Shutdown();
    
    return 0;
}

// Консольная версия для отладки
#ifdef _DEBUG
int main(int argc, char* argv[]) {
    return WinMain(GetModuleHandle(nullptr), nullptr, GetCommandLineA(), SW_SHOWDEFAULT);
}
#endif

#else
// Linux/Mac версия (заглушка)
#include <iostream>

int main(int argc, char* argv[]) {
    std::cout << "Forensic Scanner" << std::endl;
    std::cout << "Эта программа предназначена только для Windows." << std::endl;
    return 1;
}
#endif
