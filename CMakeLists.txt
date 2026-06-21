cmake_minimum_required(VERSION 3.20)
project(perfmon VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- raylib via FetchContent ---
include(FetchContent)
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG        5.5
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(raylib)

# --- Source files ---
set(SOURCES
    src/main.cpp
    src/BarWidget.cpp
    src/GaugeWidget.cpp
    src/StatsCollector.cpp
    src/ConfigManager.cpp
    src/CsvLogger.cpp
    src/TitleBar.cpp
    src/TitleBarWin32.cpp
    src/DiagnosticsOverlay.cpp
    src/SettingsOverlay.cpp
    src/LandingPage.cpp
    src/CpuMonitor.cpp
    src/GpuMonitor.cpp
    src/RamMonitor.cpp
    src/DiskMonitor.cpp
    src/NetworkMonitor.cpp
)

set(HEADERS
    src/BarWidget.h
    src/GaugeWidget.h
    src/StatsCollector.h
    src/ConfigManager.h
    src/CsvLogger.h
    src/TitleBar.h
    src/DiagnosticsOverlay.h
    src/SettingsOverlay.h
    src/LandingPage.h
    src/AppTheme.h
    src/GridLayout.h
    src/StatsData.h
    src/CpuMonitor.h
    src/GpuMonitor.h
    src/RamMonitor.h
    src/DiskMonitor.h
    src/NetworkMonitor.h
)

# --- Executable ---
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_include_directories(${PROJECT_NAME} PRIVATE src)

target_link_libraries(${PROJECT_NAME} PRIVATE raylib pdh ws2_32 psapi dwmapi dxgi d3d11 wbemuuid ole32)

# Windows subsystem to avoid console window in release
set_target_properties(${PROJECT_NAME} PROPERTIES
    WIN32_EXECUTABLE FALSE
)

# --- Test executable ---
set(TEST_SOURCES
    src/tests/test_main.cpp
    src/tests/test_cpu.cpp
    src/tests/test_ram.cpp
    src/tests/test_disk.cpp
    src/tests/test_gpu.cpp
    src/tests/test_network.cpp
    src/tests/test_graphics.cpp
    src/BarWidget.cpp
    src/GaugeWidget.cpp
    src/StatsCollector.cpp
    src/CpuMonitor.cpp
    src/GpuMonitor.cpp
    src/RamMonitor.cpp
    src/DiskMonitor.cpp
    src/NetworkMonitor.cpp
)

add_executable(perfmon_tests ${TEST_SOURCES} ${HEADERS})
target_include_directories(perfmon_tests PRIVATE src src/tests)
target_link_libraries(perfmon_tests PRIVATE raylib pdh ws2_32 psapi dxgi d3d11 wbemuuid ole32)
