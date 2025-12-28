#include <QApplication>
#include "../UI/MainWindow.h"
#include "Config.h"
#include "Logger.h"
#include "HashScanner.h"
#include "BrowserScanner.h"
#include "DiscordScanner.h"
#include <iostream>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    // Инициализация конфигурации
    Config& config = Config::getInstance();
    config.load();
    
    // Инициализация логгера
    Logger::getInstance().setLogFile("scan_log.txt");
    
    // Создание и отображение главного окна
    MainWindow window;
    window.show();
    
    return app.exec();
}
