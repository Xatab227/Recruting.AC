# Быстрый старт - Сборка EXE файла

## Что было создано

Полностью рабочая программа Forensic Scanner со всеми модулями согласно ТЗ:

✅ **Модули сканирования:**
- HashScanner - анализ хешей файлов
- BrowserScanner - проверка истории браузера
- DiscordScanner - анализ Discord активности

✅ **Графический интерфейс:**
- Главное окно с прогресс-баром
- Индикатор риска с цветовой индикацией (зеленый/желтый/красный)
- Три вкладки с логами (хеши, браузер, Discord)
- Сводная информация о триггерах

✅ **Дополнительные модули:**
- Config - управление конфигурацией
- Logger - логирование результатов
- RiskCalculator - расчет общего риска

## Как собрать EXE файл

### Вариант 1: Автоматическая сборка (Windows)

1. Откройте командную строку в папке проекта
2. Запустите:
   ```batch
   build.bat
   ```
3. После сборки exe будет в `build\Release\ForensicScanner.exe`

### Вариант 2: Ручная сборка

#### Шаг 1: Установите зависимости

**Qt6:**
- Скачайте с https://www.qt.io/download
- Установите модули: Qt6::Core, Qt6::Widgets

**CMake:**
- Скачайте с https://cmake.org/download/
- Или через chocolatey: `choco install cmake`

**Компилятор:**
- Visual Studio 2019+ (с компонентом "Desktop development with C++")
- Или MinGW-w64

#### Шаг 2: Соберите проект

```bash
# Создайте папку для сборки
mkdir build
cd build

# Настройте CMake (укажите путь к Qt6)
cmake .. -G "Visual Studio 17 2022" -A x64 -DQt6_DIR="C:/Qt/6.x.x/msvc2019_64/lib/cmake/Qt6"

# Соберите Release версию
cmake --build . --config Release
```

#### Шаг 3: Скопируйте зависимости

После сборки нужно скопировать DLL Qt6 рядом с exe файлом:

```bash
# Используйте windeployqt (находится в bin Qt6)
windeployqt.exe build\Release\ForensicScanner.exe
```

Или скопируйте вручную из `C:\Qt\6.x.x\msvc2019_64\bin\`:
- Qt6Core.dll
- Qt6Gui.dll
- Qt6Widgets.dll
- И соответствующие плагины

#### Шаг 4: Скопируйте конфигурацию

Убедитесь, что папка `config` находится рядом с exe файлом:
```
build/Release/
├── ForensicScanner.exe
└── config/
    ├── config.json
    └── hash_database.txt
```

## Запуск программы

1. Запустите `ForensicScanner.exe`
2. Нажмите "Начать сканирование"
3. Дождитесь завершения
4. Просмотрите результаты в логах

## Настройка

Отредактируйте `config/config.json` для изменения:
- Директорий сканирования
- Ключевых слов
- Подозрительных доменов
- Весов риска

Добавьте хеши в `config/hash_database.txt` (SHA-256, по одному на строку).

## Структура файлов

```
/workspace/
├── src/                    # Исходный код
│   ├── main.cpp           # Точка входа
│   ├── Config.*           # Конфигурация
│   ├── Logger.*           # Логирование
│   ├── HashScanner.*      # Сканер хешей
│   ├── BrowserScanner.*   # Сканер браузера
│   ├── DiscordScanner.*   # Сканер Discord
│   └── RiskCalculator.*  # Расчет риска
├── UI/                    # Графический интерфейс
│   ├── MainWindow.h
│   └── MainWindow.cpp
├── config/                # Конфигурация
│   ├── config.json
│   └── hash_database.txt
├── CMakeLists.txt         # Файл сборки
├── build.bat              # Скрипт автоматической сборки
├── BUILD_INSTRUCTIONS.md  # Подробная инструкция
└── README.md              # Описание проекта
```

## Решение проблем

**Ошибка "Could not find Qt6":**
- Укажите путь к Qt6 через `-DQt6_DIR` в CMake
- Пример: `-DQt6_DIR="C:/Qt/6.5.0/msvc2019_64/lib/cmake/Qt6"`

**Программа не запускается:**
- Убедитесь, что DLL Qt6 находятся в PATH или рядом с exe
- Используйте `windeployqt` для автоматического копирования

**Ошибки компиляции:**
- Убедитесь, что используется C++17 компилятор
- Проверьте, что установлен Windows SDK

## Готово!

После успешной сборки у вас будет готовый `ForensicScanner.exe` файл, который можно запускать на Windows x64 системах.
