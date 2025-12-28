# Инструкция по сборке Forensic Scanner

## Требования

1. **CMake** версии 3.16 или выше
2. **Qt6** (Core и Widgets модули)
3. **Компилятор C++17** (MSVC, MinGW или Clang)
4. **Windows SDK** (для Windows API)

## Установка зависимостей

### Windows

1. Установите Qt6:
   - Скачайте Qt6 с официального сайта: https://www.qt.io/download
   - Или используйте установщик Qt Online Installer
   - Убедитесь, что установлены модули Qt6::Core и Qt6::Widgets

2. Установите CMake:
   - Скачайте с https://cmake.org/download/
   - Или используйте установщик через chocolatey: `choco install cmake`

3. Установите компилятор:
   - **MSVC**: Установите Visual Studio 2019 или выше с компонентом "Desktop development with C++"
   - **MinGW**: Скачайте MinGW-w64 или используйте через MSYS2

## Сборка проекта

### Вариант 1: Использование CMake GUI

1. Откройте CMake GUI
2. Укажите путь к исходникам (source code): `/workspace`
3. Укажите путь для сборки (build): например, `/workspace/build`
4. Нажмите "Configure"
5. Выберите генератор (Visual Studio, MinGW Makefiles и т.д.)
6. Укажите путь к Qt6 (переменная `Qt6_DIR`)
7. Нажмите "Generate"
8. Нажмите "Open Project" или откройте проект в Visual Studio
9. Соберите проект (Build -> Build Solution)

### Вариант 2: Использование командной строки

#### Для Visual Studio (MSVC):

```bash
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64 -DQt6_DIR="C:/Qt/6.x.x/msvc2019_64/lib/cmake/Qt6"
cmake --build . --config Release
```

#### Для MinGW:

```bash
mkdir build
cd build
cmake .. -G "MinGW Makefiles" -DQt6_DIR="C:/Qt/6.x.x/mingw_64/lib/cmake/Qt6"
cmake --build . --config Release
```

## Расположение исполняемого файла

После успешной сборки исполняемый файл будет находиться в:
- **Release конфигурация**: `build/Release/ForensicScanner.exe` (для MSVC) или `build/ForensicScanner.exe` (для MinGW)
- **Debug конфигурация**: `build/Debug/ForensicScanner.exe`

## Запуск программы

1. Убедитесь, что папка `config` находится рядом с исполняемым файлом
2. Запустите `ForensicScanner.exe`
3. Нажмите кнопку "Начать сканирование"

## Структура проекта

```
/workspace/
├── src/              # Исходный код
│   ├── main.cpp
│   ├── Config.*
│   ├── Logger.*
│   ├── HashScanner.*
│   ├── BrowserScanner.*
│   ├── DiscordScanner.*
│   └── RiskCalculator.*
├── UI/               # Графический интерфейс
│   ├── MainWindow.h
│   └── MainWindow.cpp
├── config/           # Конфигурационные файлы
│   ├── config.json
│   └── hash_database.txt
├── CMakeLists.txt    # Файл сборки CMake
└── BUILD_INSTRUCTIONS.md
```

## Настройка конфигурации

Отредактируйте файл `config/config.json` для настройки:
- Директорий для сканирования
- Ключевых слов для поиска
- Подозрительных доменов
- Весов риска

База хешей находится в `config/hash_database.txt`. Добавьте хеши SHA-256 известных читов (по одному на строку).

## Решение проблем

### Ошибка: "Could not find Qt6"
- Убедитесь, что Qt6 установлен
- Укажите правильный путь к Qt6 через переменную `Qt6_DIR` в CMake

### Ошибка компиляции с Windows API
- Убедитесь, что установлен Windows SDK
- Проверьте, что используете правильный генератор для вашей системы

### Программа не запускается
- Убедитесь, что все DLL Qt6 находятся в PATH или рядом с exe файлом
- Используйте `windeployqt` для автоматического копирования зависимостей:
  ```bash
  windeployqt.exe ForensicScanner.exe
  ```

## Создание установщика (опционально)

Для создания установщика можно использовать:
- **NSIS** (Nullsoft Scriptable Install System)
- **Inno Setup**
- **Qt Installer Framework**
