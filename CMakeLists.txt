cmake_minimum_required(VERSION 3.16)
project(ForensicScanner)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Поиск Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# Включение автоматической обработки MOC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Исходные файлы
set(SOURCES
    src/main.cpp
    src/Config.cpp
    src/Logger.cpp
    src/HashScanner.cpp
    src/BrowserScanner.cpp
    src/DiscordScanner.cpp
    src/RiskCalculator.cpp
    UI/MainWindow.cpp
)

set(HEADERS
    src/Config.h
    src/Logger.h
    src/HashScanner.h
    src/BrowserScanner.h
    src/DiscordScanner.h
    src/RiskCalculator.h
    UI/MainWindow.h
)

# Создание исполняемого файла
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Подключение Qt6
target_link_libraries(${PROJECT_NAME} 
    Qt6::Core 
    Qt6::Widgets
)

# Windows специфичные настройки
if(WIN32)
    # Связывание с Windows библиотеками
    target_link_libraries(${PROJECT_NAME} 
        advapi32
    )
    
    # Установка подсистемы Windows
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

# Копирование конфигурационных файлов
file(COPY ${CMAKE_SOURCE_DIR}/config DESTINATION ${CMAKE_BINARY_DIR})
