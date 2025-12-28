cmake_minimum_required(VERSION 3.16)
project(ForensicScanner VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Windows-specific settings
if(WIN32)
    add_definitions(-D_UNICODE -DUNICODE)
    add_definitions(-DWIN32_LEAN_AND_MEAN)
    add_definitions(-DNOMINMAX)
endif()

# ImGui source files (будут скачаны)
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/libs/imgui)

# Source files
set(SOURCES
    src/main.cpp
    src/Config.cpp
    src/Logger.cpp
    src/HashScanner.cpp
    src/BrowserScanner.cpp
    src/DiscordScanner.cpp
    src/RiskCalculator.cpp
    src/UI/MainWindow.cpp
    src/Utils.cpp
)

set(HEADERS
    src/Config.h
    src/Logger.h
    src/HashScanner.h
    src/BrowserScanner.h
    src/DiscordScanner.h
    src/RiskCalculator.h
    src/UI/MainWindow.h
    src/Utils.h
    src/Types.h
)

# ImGui sources
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_win32.cpp
    ${IMGUI_DIR}/backends/imgui_impl_dx11.cpp
)

# Create executable
add_executable(${PROJECT_NAME} WIN32
    ${SOURCES}
    ${HEADERS}
    ${IMGUI_SOURCES}
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    d3d11
    d3dcompiler
    dxgi
    bcrypt
    shlwapi
    dwmapi
)

# Copy config files to build directory
file(COPY ${CMAKE_SOURCE_DIR}/config DESTINATION ${CMAKE_BINARY_DIR})

# Set output directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
