# CMakeLists.txt для Forensic Scanner
# Минимальная версия CMake
cmake_minimum_required(VERSION 3.16)

# Название проекта
project(ForensicScanner VERSION 1.0.0 LANGUAGES CXX)

# Стандарт C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Только Windows
if(NOT WIN32)
    message(FATAL_ERROR "This project is Windows-only")
endif()

# Настройки компиляции
if(MSVC)
    # Отключаем предупреждения о небезопасных функциях
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    # Unicode
    add_definitions(-DUNICODE -D_UNICODE)
    # Статическая линковка runtime
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# Пути к исходникам
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(LIBS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs)

# Исходные файлы
set(SOURCES
    ${SRC_DIR}/main.cpp
    ${SRC_DIR}/Config.cpp
    ${SRC_DIR}/Logger.cpp
    ${SRC_DIR}/HashScanner.cpp
    ${SRC_DIR}/BrowserScanner.cpp
    ${SRC_DIR}/DiscordScanner.cpp
    ${SRC_DIR}/RiskCalculator.cpp
    ${SRC_DIR}/UI/MainWindow.cpp
)

# Заголовочные файлы
set(HEADERS
    ${INCLUDE_DIR}/Config.h
    ${INCLUDE_DIR}/Logger.h
    ${INCLUDE_DIR}/HashScanner.h
    ${INCLUDE_DIR}/BrowserScanner.h
    ${INCLUDE_DIR}/DiscordScanner.h
    ${INCLUDE_DIR}/RiskCalculator.h
    ${INCLUDE_DIR}/MainWindow.h
)

# ImGui файлы (нужно скачать из https://github.com/ocornut/imgui)
set(IMGUI_DIR ${LIBS_DIR}/imgui)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_impl_win32.cpp
    ${IMGUI_DIR}/imgui_impl_dx11.cpp
)

# Создаём исполняемый файл
add_executable(${PROJECT_NAME} WIN32 
    ${SOURCES}
    ${HEADERS}
    ${IMGUI_SOURCES}
)

# Директории с заголовками
target_include_directories(${PROJECT_NAME} PRIVATE
    ${INCLUDE_DIR}
    ${IMGUI_DIR}
)

# Системные библиотеки Windows
target_link_libraries(${PROJECT_NAME} PRIVATE
    d3d11
    dxgi
    d3dcompiler
    dwmapi
    advapi32
    ole32
    oleaut32
    uuid
)

# Копируем конфигурационные файлы в директорию сборки
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/config
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/config
)

# Установка
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/config DESTINATION bin)

# Информация о сборке
message(STATUS "")
message(STATUS "=== Forensic Scanner Build Configuration ===")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "")
