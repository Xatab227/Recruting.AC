@echo off
setlocal enabledelayedexpansion

echo ========================================
echo   Сборка Forensic Scanner
echo ========================================
echo.

:: Проверяем наличие ImGui
if not exist "%~dp0libs\imgui\imgui.cpp" (
    echo ОШИБКА: ImGui не найден!
    echo.
    echo Запустите setup_imgui.bat для загрузки ImGui
    echo или скачайте вручную с https://github.com/ocornut/imgui
    echo.
    pause
    exit /b 1
)

:: Проверяем наличие CMake
where cmake >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo ОШИБКА: CMake не установлен или не в PATH
    echo Установите CMake с https://cmake.org/
    pause
    exit /b 1
)

:: Определяем генератор Visual Studio
set VS_GENERATOR=

:: Проверяем VS 2022
if exist "%ProgramFiles%\Microsoft Visual Studio\2022" (
    set VS_GENERATOR=Visual Studio 17 2022
    goto :found_vs
)
if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2022" (
    set VS_GENERATOR=Visual Studio 17 2022
    goto :found_vs
)

:: Проверяем VS 2019
if exist "%ProgramFiles%\Microsoft Visual Studio\2019" (
    set VS_GENERATOR=Visual Studio 16 2019
    goto :found_vs
)
if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2019" (
    set VS_GENERATOR=Visual Studio 16 2019
    goto :found_vs
)

echo ОШИБКА: Visual Studio 2019 или 2022 не найдена
pause
exit /b 1

:found_vs
echo Найден: %VS_GENERATOR%
echo.

:: Создаём директорию сборки
set BUILD_DIR=%~dp0build
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
cd /d "%BUILD_DIR%"

:: Генерируем проект
echo Генерация проекта CMake...
cmake .. -G "%VS_GENERATOR%" -A x64

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ОШИБКА: Генерация CMake завершилась с ошибкой
    pause
    exit /b 1
)

echo.
echo Сборка Release...
cmake --build . --config Release

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ОШИБКА: Сборка завершилась с ошибкой
    pause
    exit /b 1
)

:: Проверяем результат
set EXE_PATH=%BUILD_DIR%\Release\ForensicScanner.exe
if exist "%EXE_PATH%" (
    echo.
    echo ========================================
    echo   Сборка успешно завершена!
    echo ========================================
    echo.
    echo Исполняемый файл: %EXE_PATH%
    echo.
    
    :: Копируем в корень для удобства
    copy "%EXE_PATH%" "%~dp0ForensicScanner.exe" >nul
    
    :: Копируем конфиг рядом с exe если его нет
    if not exist "%~dp0config" (
        xcopy /E /I "%~dp0config" "%~dp0config" >nul
    )
    
    echo Копия создана: %~dp0ForensicScanner.exe
    echo.
) else (
    echo.
    echo ПРЕДУПРЕЖДЕНИЕ: EXE файл не найден
    echo Проверьте директорию: %BUILD_DIR%\Release\
)

echo.
pause
