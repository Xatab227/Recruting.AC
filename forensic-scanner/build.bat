@echo off
chcp 65001 >nul
echo ========================================
echo   Сборка Forensic Scanner
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

:: Определяем компилятор
set COMPILER=NONE

:: Проверяем Visual Studio 2022
if exist "%ProgramFiles%\Microsoft Visual Studio\2022" set COMPILER=VS2022
if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2022" set COMPILER=VS2022

:: Проверяем Visual Studio 2019
if "%COMPILER%"=="NONE" (
    if exist "%ProgramFiles%\Microsoft Visual Studio\2019" set COMPILER=VS2019
    if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2019" set COMPILER=VS2019
)

:: Проверяем MinGW
if "%COMPILER%"=="NONE" (
    where g++ >nul 2>nul
    if %ERRORLEVEL% EQU 0 set COMPILER=MINGW
)

:: Нет компилятора
if "%COMPILER%"=="NONE" (
    echo ОШИБКА: Компилятор не найден!
    echo.
    echo Установите один из вариантов:
    echo.
    echo 1. Visual Studio 2019/2022 (Community - бесплатно)
    echo    https://visualstudio.microsoft.com/
    echo.
    echo 2. MSYS2 + MinGW (бесплатно, проще)
    echo    https://www.msys2.org/
    echo    После установки в MSYS2:
    echo    pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake
    echo    Добавьте C:\msys64\mingw64\bin в PATH
    echo.
    pause
    exit /b 1
)

echo Найден компилятор: %COMPILER%
echo.

:: Создаём директорию сборки
set BUILD_DIR=%~dp0build
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
cd /d "%BUILD_DIR%"

:: Сборка в зависимости от компилятора
if "%COMPILER%"=="VS2022" (
    echo Генерация для Visual Studio 2022...
    cmake .. -G "Visual Studio 17 2022" -A x64
    if %ERRORLEVEL% NEQ 0 goto :error
    echo Сборка Release...
    cmake --build . --config Release
    if %ERRORLEVEL% NEQ 0 goto :error
    set EXE_PATH=%BUILD_DIR%\Release\ForensicScanner.exe
    goto :success
)

if "%COMPILER%"=="VS2019" (
    echo Генерация для Visual Studio 2019...
    cmake .. -G "Visual Studio 16 2019" -A x64
    if %ERRORLEVEL% NEQ 0 goto :error
    echo Сборка Release...
    cmake --build . --config Release
    if %ERRORLEVEL% NEQ 0 goto :error
    set EXE_PATH=%BUILD_DIR%\Release\ForensicScanner.exe
    goto :success
)

if "%COMPILER%"=="MINGW" (
    echo Генерация для MinGW...
    cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
    if %ERRORLEVEL% NEQ 0 goto :error
    echo Сборка...
    mingw32-make -j4
    if %ERRORLEVEL% NEQ 0 goto :error
    set EXE_PATH=%BUILD_DIR%\ForensicScanner.exe
    goto :success
)

:error
echo.
echo ========================================
echo   ОШИБКА СБОРКИ
echo ========================================
pause
exit /b 1

:success
echo.
echo ========================================
echo   УСПЕХ! Сборка завершена!
echo ========================================
echo.

if exist "%EXE_PATH%" (
    copy "%EXE_PATH%" "%~dp0ForensicScanner.exe" >nul
    echo Файл: %~dp0ForensicScanner.exe
) else (
    echo EXE: %EXE_PATH%
)
echo.
pause
