@echo off
echo ========================================
echo   Загрузка библиотеки ImGui
echo ========================================
echo.

:: Проверяем наличие git
where git >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo ОШИБКА: Git не установлен или не в PATH
    echo Установите Git с https://git-scm.com/
    pause
    exit /b 1
)

:: Создаём временную директорию
set TEMP_DIR=%~dp0temp_imgui
if exist "%TEMP_DIR%" rmdir /s /q "%TEMP_DIR%"
mkdir "%TEMP_DIR%"

echo Клонирование ImGui...
git clone --depth 1 https://github.com/ocornut/imgui.git "%TEMP_DIR%"

if %ERRORLEVEL% NEQ 0 (
    echo ОШИБКА: Не удалось клонировать репозиторий
    pause
    exit /b 1
)

:: Целевая директория
set TARGET_DIR=%~dp0libs\imgui

:: Создаём директорию если не существует
if not exist "%TARGET_DIR%" mkdir "%TARGET_DIR%"

echo.
echo Копирование файлов...

:: Копируем основные файлы
copy "%TEMP_DIR%\imgui.cpp" "%TARGET_DIR%\"
copy "%TEMP_DIR%\imgui.h" "%TARGET_DIR%\"
copy "%TEMP_DIR%\imgui_demo.cpp" "%TARGET_DIR%\"
copy "%TEMP_DIR%\imgui_draw.cpp" "%TARGET_DIR%\"
copy "%TEMP_DIR%\imgui_internal.h" "%TARGET_DIR%\"
copy "%TEMP_DIR%\imgui_tables.cpp" "%TARGET_DIR%\"
copy "%TEMP_DIR%\imgui_widgets.cpp" "%TARGET_DIR%\"
copy "%TEMP_DIR%\imconfig.h" "%TARGET_DIR%\"
copy "%TEMP_DIR%\imstb_rectpack.h" "%TARGET_DIR%\"
copy "%TEMP_DIR%\imstb_textedit.h" "%TARGET_DIR%\"
copy "%TEMP_DIR%\imstb_truetype.h" "%TARGET_DIR%\"

:: Копируем бэкенды
copy "%TEMP_DIR%\backends\imgui_impl_win32.cpp" "%TARGET_DIR%\"
copy "%TEMP_DIR%\backends\imgui_impl_win32.h" "%TARGET_DIR%\"
copy "%TEMP_DIR%\backends\imgui_impl_dx11.cpp" "%TARGET_DIR%\"
copy "%TEMP_DIR%\backends\imgui_impl_dx11.h" "%TARGET_DIR%\"

:: Удаляем временную директорию
rmdir /s /q "%TEMP_DIR%"

echo.
echo ========================================
echo   ImGui успешно установлен!
echo ========================================
echo.
echo Теперь запустите build.bat для сборки проекта.
echo.
pause
