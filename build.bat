@echo off
REM Скрипт для сборки проекта на Windows

echo Creating build directory...
if not exist build mkdir build
cd build

echo Configuring CMake...
cmake .. -G "Visual Studio 17 2022" -A x64

if %ERRORLEVEL% NEQ 0 (
    echo CMake configuration failed!
    echo Trying MinGW...
    cmake .. -G "MinGW Makefiles"
)

if %ERRORLEVEL% NEQ 0 (
    echo Build configuration failed!
    pause
    exit /b 1
)

echo Building project...
cmake --build . --config Release

if %ERRORLEVEL% EQU 0 (
    echo.
    echo Build successful!
    echo Executable location: build\Release\ForensicScanner.exe
) else (
    echo Build failed!
)

pause
