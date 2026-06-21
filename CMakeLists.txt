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
    src/BarV1.cpp
    src/GaugeV1.cpp
    src/StatsV1.cpp
    src/ConfigV1.cpp
    src/LoggerV1.cpp
)

set(HEADERS
    src/BarV1.h
    src/GaugeV1.h
    src/StatsV1.h
    src/ConfigV1.h
    src/LoggerV1.h
    src/TileV1.h
    src/ThemeV1.h
    src/Rendering.h
    src/LandingPage.h
    src/Overlays.h
    src/DataTypes.h
    src/WindowManager.h
)

# --- Executable ---
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_include_directories(${PROJECT_NAME} PRIVATE src)

target_link_libraries(${PROJECT_NAME} PRIVATE raylib pdh ws2_32 psapi)

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
    src/BarV1.cpp
    src/GaugeV1.cpp
    src/StatsV1.cpp
)

add_executable(perfmon_tests ${TEST_SOURCES} ${HEADERS})
target_include_directories(perfmon_tests PRIVATE src src/tests)
target_link_libraries(perfmon_tests PRIVATE raylib pdh ws2_32 psapi)
