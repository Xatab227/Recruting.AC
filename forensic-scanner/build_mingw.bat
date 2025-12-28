@echo off
chcp 65001 >nul
echo ========================================
echo   Сборка Forensic Scanner (MinGW)
echo ========================================
echo.

:: Проверяем наличие ImGui
if not exist "%~dp0libs\imgui\imgui.cpp" (
    echo ОШИБКА: ImGui не найден!
    echo.
    echo Запустите setup_imgui.bat для загрузки ImGui
    pause
    exit /b 1
)

:: Проверяем CMake
where cmake >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo ОШИБКА: CMake не найден!
    echo Установите CMake: https://cmake.org/download/
    pause
    exit /b 1
)

:: Проверяем MinGW
where g++ >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo ОШИБКА: MinGW (g++) не найден!
    echo.
    echo Установите один из вариантов:
    echo.
    echo 1. MSYS2 (рекомендуется):
    echo    https://www.msys2.org/
    echo    После установки выполните в MSYS2:
    echo    pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake
    echo.
    echo 2. MinGW-w64:
    echo    https://winlibs.com/
    echo    Скачайте версию с UCRT runtime
    echo.
    echo Добавьте путь к bin в PATH (например C:\msys64\mingw64\bin)
    pause
    exit /b 1
)

echo Найден компилятор:
g++ --version | findstr /C:"g++"
echo.

:: Создаём директорию сборки
set BUILD_DIR=%~dp0build
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
cd /d "%BUILD_DIR%"

:: Генерируем Makefile
echo Генерация проекта...
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ОШИБКА при генерации CMake
    pause
    exit /b 1
)

:: Собираем
echo.
echo Сборка...
mingw32-make -j4

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ОШИБКА при сборке
    pause
    exit /b 1
)

:: Проверяем результат
if exist "%BUILD_DIR%\ForensicScanner.exe" (
    echo.
    echo ========================================
    echo   УСПЕХ! Сборка завершена!
    echo ========================================
    echo.
    copy "%BUILD_DIR%\ForensicScanner.exe" "%~dp0ForensicScanner.exe" >nul
    echo Файл: %~dp0ForensicScanner.exe
    echo.
) else (
    echo.
    echo EXE не найден. Проверьте ошибки выше.
)

pause
