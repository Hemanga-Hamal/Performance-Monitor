# AI Agent Guide — Performance Monitor

## Overview
Windows-only C++ desktop app that displays real-time system performance metrics using raylib for rendering and the Windows PDH (Performance Data Helper) API for data collection.

## Tech Stack
- **Language:** C++17
- **Graphics:** raylib 5.5 (fetched via CMake FetchContent)
- **System APIs:** PDH (pdh.lib), Win32 (kernel32), Winsock (ws2_32)
- **Build:** CMake 3.20+ (primary) or MinGW Makefile (legacy)

## File Map

```
Project root
├── CMakeLists.txt          # Primary build system (FetchContent raylib 5.5)
├── Makefile                # Legacy MinGW build (hardcoded raylib path)
├── .gitignore              # Ignores build/, *.exe, *.o, IDE files
├── PLAN.md                 # High-level project roadmap
├── TODO.md                 # Granular task list for AI agents
├── README.md               # User-facing build/usage docs
├── resources/
│   └── LICENSE             # Project license
└── src/
    ├── main.cpp            # Entry point, landing page, tile dashboard, F2/F3 overlays
    ├── StatsV1.cpp/.h      # PDH + Win32 stats collector (CPU, RAM, GPU, Disk, Network)
    ├── BarV1.cpp/.h        # Horizontal progress bar raylib widget
    ├── GaugeV1.cpp/.h      # Circular arc gauge raylib widget
    ├── TileV1.h            # Tile widget (grid layout, drag, resize, snap, collision avoidance)
    ├── ThemeV1.h           # Color theme struct (Dark, Light, High Contrast presets)
    └── tests/
        ├── test_main.cpp       # Test runner (63 tests, 7 groups)
        ├── test_harness.h      # Shared test macros (TEST, CHECK, CHECK_EQ, CHECK_RANGE)
        ├── test_cpu.cpp        # CPU tests (frequency, utilization, model name)
        ├── test_ram.cpp        # RAM tests (total, used, utilization)
        ├── test_gpu.cpp        # GPU tests (available, utilization, model, name)
        ├── test_disk.cpp       # Disk tests (count, name, total, used, utilization)
        ├── test_network.cpp    # Network tests (WiFi/Ethernet send/receive, adapter list)
        └── test_graphics.cpp   # Widget tests (BarV1: theme/dims/config/sizing; GaugeV1: clamp/presets/setters)
```

## Build Commands

### CMake (recommended)
```powershell
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```
Output: `build\Release\perfmon.exe`

### Tests
```powershell
cmake --build build --config Release --target perfmon_tests
.\build\Release\perfmon_tests.exe
```

### Clean
```powershell
Remove-Item -Recurse -Force build
```

## Coding Conventions

### Includes
- Windows headers first, then raylib, then standard library
- Use `#ifndef HEADER_H` / `#define HEADER_H` / `#endif` include guards — NOT `#pragma once`
- In .cpp files that include raylib, Windows defines come BEFORE raylib:
  ```cpp
  #define WIN32_LEAN_AND_MEAN
  #define NOMINMAX
  #define NOGDI
  #define NOUSER
  #include <windows.h>
  #include "raylib.h"
  ```

### Naming
- Classes: PascalCase (StatsV1, BarV1, GaugeV1, TileV1)
- Methods: camelCase (setValue, calculateBarSize)
- PDH getters: ALLCAPS (GETCPUFrequency, GETRAMUsed)
- Members: camelCase (cpuQuery, barWidth)
- Structs: PascalCase nested inside classes (Theme, Dimensions, Config)

### Code style
- No trailing comments on code lines
- Prefer `//` comments over `/* */`
- Use `noexcept` on all methods that can't throw
- Use `[[nodiscard]]` on getters
- Delete copy ctor/assign, default move for RAII classes
- Use `constexpr` for compile-time constants in anonymous namespace

### Error handling
- PDH failures: return 0.0f or false silently
- Win32 failures: same pattern — return 0.0f or empty string
- Do NOT add logging, exceptions, or error messages unless explicitly asked

## Key Types

| Type | Location | Purpose |
|------|----------|---------|
| `StatsV1` | StatsV1.h | Stats collector (CPU/RAM/GPU/Disk/Network) |
| `StatsV1::DiskInfo` | StatsV1.h | Per-disk info (name, total GB, used GB, utilization) |
| `StatsV1::NetworkCounters` | StatsV1.h (private) | PDH handles + cached rates + timing for one interface |
| `BarV1` | BarV1.h | Horizontal progress bar widget |
| `BarV1::Theme` | BarV1.h | Bar colors (background, foreground, text) |
| `BarV1::Dimensions` | BarV1.h | Bar sizing (width, height, scalingRatio, minSize, maxSize) |
| `BarV1::Config` | BarV1.h | Bar value config (value, maxValue, autoScale, screenSizeRatio) |
| `GaugeV1` | GaugeV1.h | Circular arc gauge widget |
| `GaugeV1::Theme` | GaugeV1.h | Gauge colors (bg, arcBg, arcActive, text) |
| `GaugeV1::Dimensions` | GaugeV1.h | Gauge sizing (baseSize, scaleRatio, arcThickness, minSize, maxSize) |
| `GaugeV1::Config` | GaugeV1.h | Gauge config (startAngle, totalAngle, autoScale, screenSizeRatio) |
| `GaugeV1::Config::ConfigArc()` | GaugeV1.h | 240 degree arc preset |
| `GaugeV1::Config::ConfigQuarter()` | GaugeV1.h | 270 degree quarter preset |
| `TileV1` | TileV1.h | Tile widget (bounds, drag, resize, snap, collision avoidance) |
| `TileConfig` | TileV1.h | Tile grid position (col, row, spanCols, spanRows, title) |
| `ThemeV1` | ThemeV1.h | App-wide color theme (Dark, Light, HighContrast presets) |
| `StatsData` | main.cpp | Atomic wrappers for cross-thread stats sharing |

## UI Architecture

### State machine
`enum AppState { LANDING, DASHBOARD }` — starts on landing page, transitions to dashboard on "Start Monitoring" click.

### Tile system
- 5 tiles in a 2-column, 4-row grid: CPU(0,0), RAM(1,0), GPU(0,1), Network(1,1), Storage(0,2 spanCols=2 spanRows=2)
- `computeBounds()` — initial grid layout (skipped if tile already positioned via `isDirty`)
- `handleDrag()` — mouse-driven drag/resize with snap preview
- `snapToGrid()` — on release, snaps to nearest cell and resizes to cell dimensions; pushes overlapped tiles out of the way
- `drawFrame()` — rounded rectangle with title bar, separator line, resize grip triangle
- Content auto-scales: `gaugeCPU.setBaseSize(tileSize * 0.5f)`, `barDims.maxSize = tileWidth * 0.8f`

### Overlays
- **F2** — scrollable diagnostics panel: CPU model/freq/util, RAM total/used/util, GPU model/instance/util, per-disk info, network adapter list
- **F3** — device settings panel: per-category toggle with device-specific labels
- **Landing page** — theme picker (3 cards with color swatches), "Start Monitoring" button

## Working with PDH

### Counter path format
`\Category(Instance)\Counter` — e.g. `\Processor Information(_Total)\% Processor Performance`

### Rate counters need two collects
```cpp
PdhCollectQueryData(query);              // prime
// ... wait 200ms+ ...
PdhCollectQueryData(query);              // second collect
PdhGetFormattedCounterValue(counter, PDH_FMT_DOUBLE, nullptr, &val);  // read rate
```

### Timing pattern (non-blocking)
All stats collectors use `QueryPerformanceCounter` for timing. On first call: prime + return cached. On subsequent calls: check elapsed time; if < minimum interval, return cached; otherwise collect + compute + cache + return.

### Key PDH object names
| Object | Instance | Counter |
|--------|----------|---------|
| Processor Information | _Total | % Processor Performance, Processor Frequency |
| GPU Engine | gpu_0 (etc, skip _Total) | Utilization Percentage |
| Network Interface | (adapter name) | Bytes Sent/sec, Bytes Received/sec |

## Constraints
- Windows 10+ only
- No third-party dependencies beyond CMakeLists.txt
- raylib 5.5 fetched via FetchContent at build time
- Do NOT commit build artifacts (*.exe, *.o, build/, .opencode/)
- Do NOT add Python scripts or requirements
- Keep files focused — each .h/.cpp pair handles one concern
