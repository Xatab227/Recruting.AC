@echo off
echo ========================================
echo   Очистка проекта
echo ========================================
echo.

set BUILD_DIR=%~dp0build

if exist "%BUILD_DIR%" (
    echo Удаление директории build...
    rmdir /s /q "%BUILD_DIR%"
    echo Директория build удалена.
) else (
    echo Директория build не найдена.
)

if exist "%~dp0ForensicScanner.exe" (
    echo Удаление ForensicScanner.exe...
    del "%~dp0ForensicScanner.exe"
)

if exist "%~dp0scan_log.txt" (
    echo Удаление scan_log.txt...
    del "%~dp0scan_log.txt"
)

echo.
echo Очистка завершена.
pause
