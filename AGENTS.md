# AI Agent Guide — Performance Monitor

## Overview
Windows-only C++17 desktop app that displays real-time system performance metrics using raylib 5.5 for rendering and the Windows PDH (Performance Data Helper) API for data collection. Features a 4x4 cell-based tile dashboard with drag-to-arrange and displacement cascade, 3 themes, config persistence, CSV logging, and a 63-test suite.

## Tech Stack
- **Language:** C++17
- **Graphics:** raylib 5.5 (fetched via CMake FetchContent)
- **System APIs:** PDH (pdh.lib), Win32 (kernel32), DXGI (dxgi.lib), WMI (wbemuuid.lib), Winsock (ws2_32)
- **Build:** CMake 3.20+

## File Map

```
Project root
├── CMakeLists.txt          # FetchContent raylib 5.5, targets: perfmon.exe + perfmon_tests.exe
├── .gitignore              # Ignores build/, *.exe, *.o, IDE files
├── PLAN.md                 # High-level project roadmap + phase history
├── TODO.md                 # Known issues + future tasks for AI agents
├── README.md               # User-facing build/usage docs
├── resources/
│   └── LICENSE             # Project license
└── src/
    ├── main.cpp            # Entry point (~10 lines): just calls WindowManager::run()
    ├── WindowManager.h     # Application orchestrator: global state, config load/save, updateStats thread,
    │                       # renderLoop (state machine, F2/F3/F4 dispatch, status bar, config save on exit)
    ├── Rendering.h         # Unified rendering module:
    │                       #   DesignSystem namespace: tileTitleFont, modelFont, makeBarTheme, makeGaugeTheme
    │                       #   StatsData struct (atomic cross-thread stats)
    │                       #   formatValue helper
    │                       #   renderCPUTile, renderRAMTile, renderGPUTile, renderNetworkTile, renderStorageTile
    ├── LandingPage.h       # Landing page UI: AppState enum, drawLandingPage (hero, 3 theme cards, Start button)
    ├── Overlays.h          # F2 diagnostics overlay, F3 settings overlay (scrollable frosted panels)
    ├── TileV1.h            # Tile grid system: TileConfig, LayoutConfig (4x4 grid constants),
    │                       # TileV1 class (cell-based occupancy, drag/resize, displacement cascade,
    │                       # ghost preview, snap-on-release, drawFrame, createDefaultTiles)
    ├── ThemeV1.h           # Color theme: Dark, Light, HighContrast presets — 22 color fields
    ├── DataTypes.h         # Shared type definitions: DiskInfo, AdapterInfo, GPUInstance
    ├── StatsV1.cpp / .h    # PDH + DXGI + WMI stats collector
    │                       # CPU: frequency (PDH), utilization (GetSystemTimes), model (registry)
    │                       # RAM: total/used/util (GlobalMemoryStatusEx)
    │                       # GPU: utilization (PDH per-engine), VRAM (PDH dedicated), total VRAM (DXGI),
    │                       #   model (DXGI adapter desc), clock speed (WMI Win32_VideoController)
    │                       # Disk: all DRIVE_FIXED via GetLogicalDrives, per-disk enabled/disabled
    │                       # Network: WiFi/Ethernet via PdhExpandWildCardPathW, keyword filter
    │                       # Timing: non-blocking QPC-timed two-phase collection
    ├── BarV1.cpp / .h      # Horizontal progress bar widget
    ├── GaugeV1.cpp / .h    # Circular arc gauge widget (300-segment triangle arc)
    ├── ConfigV1.cpp / .h   # Config persistence via INI file in %APPDATA%\PerfMon\config.ini
    ├── LoggerV1.cpp / .h   # CSV logger, F4 toggle, writes timestamped rows to working directory
    └── tests/
        ├── test_main.cpp       # Test runner (63 tests, 7 groups)
        ├── test_harness.h      # Shared macros: TEST, CHECK, CHECK_EQ, CHECK_RANGE
        ├── test_cpu.cpp        # CPU tests (frequency, utilization, model name)
        ├── test_ram.cpp        # RAM tests (total, used, utilization)
        ├── test_gpu.cpp        # GPU tests (available, utilization, model, name)
        ├── test_disk.cpp       # Disk tests (count, name, total, used, utilization)
        ├── test_network.cpp    # Network tests (WiFi/Ethernet send/receive, adapter list)
        └── test_graphics.cpp   # Widget tests (BarV1: theme/dims/config/sizing; GaugeV1: clamp/presets/setters)
```

## Build Commands

### CMake
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
- **Include guards:** `#ifndef HEADER_H` / `#define HEADER_H` / `#endif` — NOT `#pragma once`
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
- Classes: PascalCase (StatsV1, BarV1, GaugeV1, TileV1, ConfigV1, LoggerV1, WindowManager)
- Methods: camelCase (setValue, calculateBarSize, getTotalHeight)
- PDH getters: ALLCAPS (GETCPUFrequency, GETRAMUsed)
- Members: camelCase (cpuQuery, barWidth, gpuInstances, adapterInfos)
- Structs: PascalCase at namespace/global scope (DiskInfo, AdapterInfo, GPUInstance, TileConfig, LayoutConfig, StatsData, AppConfig)
- Namespaces: PascalCase (DesignSystem)

### Code style
- No trailing comments on code lines
- Prefer `//` comments over `/* */`
- Use `noexcept` on all methods that can't throw
- Use `[[nodiscard]]` on getters
- Delete copy ctor/assign, default move for RAII classes
- Use `constexpr` for compile-time constants

### Error handling
- PDH failures: return 0.0f or false silently
- Win32 failures: same pattern — return 0.0f or empty string
- Do NOT add logging, exceptions, or error messages unless explicitly asked

## Key Types

| Type | Location | Purpose |
|------|----------|---------|
| `WindowManager` | WindowManager.h | Application orchestrator: state, threads, main loop |
| `StatsV1` | StatsV1.h | Stats collector (CPU/RAM/GPU/Disk/Network) |
| `GPUInstance` | DataTypes.h | Per-GPU PDH query + counter handles, VRAM query, name, displayName, cached values |
| `DiskInfo` | DataTypes.h | Per-disk info (name, total GB, used GB, utilization, enabled) |
| `AdapterInfo` | DataTypes.h | Per-adapter info (name, isWiFi, isEthernet, enabled) |
| `StatsData` | Rendering.h | Atomic wrappers for cross-thread stats sharing (CPU, RAM, GPU util/VRAM/clock, network, disk) |
| `BarV1` | BarV1.h | Horizontal progress bar widget |
| `BarV1::Theme` | BarV1.h | Bar colors (background, foreground, text) |
| `BarV1::Dimensions` | BarV1.h | Bar sizing (width, height, scalingRatio, textSizeRatio, minSize, maxSize) |
| `BarV1::Config` | BarV1.h | Bar value config (value, maxValue, autoScale, screenSizeRatio) |
| `GaugeV1` | GaugeV1.h | Circular arc gauge widget |
| `GaugeV1::Theme` | GaugeV1.h | Gauge colors (bg, arcBg, arcActive, text) |
| `GaugeV1::Dimensions` | GaugeV1.h | Gauge sizing (baseSize, scaleRatio, arcThickness, textSizeRatio, minSize, maxSize) |
| `GaugeV1::Config` | GaugeV1.h | Gauge config: ConfigArc (240 deg), ConfigQuarter (270 deg) |
| `TileV1` | TileV1.h | Tile widget: cell-based occupancy grid, drag/resize, displacement cascade |
| `TileConfig` | TileV1.h | Tile grid position (col, row, spanCols, spanRows, minSpanCols, minSpanRows, title) |
| `LayoutConfig` | TileV1.h | Grid constants: gridCols=4, gridRows=4, statusBarH=30, tilePadding=10, maxTiles=5 |
| `ThemeV1` | ThemeV1.h | App-wide 22-field color theme (Dark, Light, HighContrast static presets) |
| `AppConfig` | ConfigV1.h | Persisted config (themeIndex, tileEnabled[5], window pos/size) |
| `ConfigV1` | ConfigV1.h | Load/save AppConfig from %APPDATA%\PerfMon\config.ini |
| `LoggerV1` | LoggerV1.h | CSV log writer (F4 toggle, timestamped rows) |
| `AppState` | LandingPage.h | State machine enum: LANDING, DASHBOARD |

## UI Architecture

### State machine
`enum AppState { LANDING, DASHBOARD }` — starts on landing page, transitions to dashboard on "Start Monitoring" click.

### Grid system (current: 4 columns × 4 rows)
```
Cell = screen / grid dimensions
Default tile layout:
  CPU:      (col=0, row=0, spanCols=2, spanRows=2)  — top-left
  RAM:      (col=0, row=2, spanCols=2, spanRows=2)  — bottom-left
  GPU:      (col=2, row=0, spanCols=2, spanRows=2)  — top-right
  Network:  (col=2, row=2, spanCols=2, spanRows=2)  — bottom-right
  Storage:  (col=3, row=0, spanCols=4, spanRows=1)  — full-width narrow row (takes remaining row)
Total tiles: 5. Tile area height: screen height - 30px status bar.
```
Grid config is in `LayoutConfig` struct inside `TileV1.h`.

### Tile system (cell-based displacement algorithm)
1. **Occupancy grid:** `buildOccupancy()` builds a bool[4][4] from all tiles' config positions
2. **Fit check:** `fitsInGrid()` tests if a config fits at (row,col) without overlap
3. **Displacement cascade:** `tryPlaceTile()` finds conflicting tiles, unmarks them, places the dragged tile first, then calls `tryShiftTile()` on each displaced tile to find the best alternative position (minimizing distance from original)
4. **Ghost preview:** `updateGhostPreview()` shows the dragged tile's projected position (accent ghost fill) plus all displaced tiles' new positions (muted ghost fill)
5. **Rejection:** If displacement fails (any displaced tile can't find a valid cell), the drag snaps back to original position
6. **Min-size enforcement:** Each tile has `minSpanCols`/`minSpanRows` — a tile-sized opening in the grid that violates these minimums is rejected

### Content per tile
- **CPU:** Model text (auto-shrink to fit width) → gauge (arc, 240°) → bar (frequency)
- **RAM:** Gauge only (quarter, 270°)
- **GPU:** Model text → 3 bars per GPU (Util%, VRAM GB, Clock MHz). VRAM bar shows used/total GB as label; clock bar maps MHz to 0-100 scale (max 3000 MHz)
- **Network:** 4 bars (WiFi Up/Down, Eth Up/Down) with compact labels
- **Storage:** N bars (one per enabled disk)

Per-tile rendering is in `Rendering.h`.

### Content sizing
- All content positioned relative to `tile.bounds` (not the global window)
- Title font: `DesignSystem::tileTitleFont(tileW, tileH)` — tile-relative
- Model text font: `DesignSystem::modelFont(tileW)` — tile-relative
- Widget themes mapped from `ThemeV1` via `DesignSystem::makeBarTheme()` / `makeGaugeTheme()`

### Overlays
- **F2 Diagnostics:** Centered frosted panel (80% × 80% screen), scissor-clipped scroll region, accent-colored section headers, scrollbar, "Save" exports diagnostics.txt
- **F3 Settings:** Right-aligned frosted panel (370px wide, 80% screen height), scissor-clipped scroll, checkbox toggles with checkmarks, per-tile / per-disk / per-adapter sections
- **F4 CSV Logging:** Toggle on/off. Writes to `perfmon_YYYYMMDD_HHMMSS.csv` in working directory

### Status bar
- 30px bar at `y = screenH - LayoutConfig::statusBarH`
- Left: "F2:Diag  F3:Settings  F4:Log" in muted text
- Right: "F4:Log [ON/OFF]" in accent color (ON) or muted (OFF)
- FPS counter at top-right

### Landing page
- Centered hero title + subtitle, 3 theme cards (Dark, Light, High Contrast) with drop shadows and hover glow, color swatch previews, "Start Monitoring" pill button
- Layout scales with `cardScale = clamp(min(sw/1300, sh/850), 0.5, 2.0)`

## Module Dependency Graph

```
main.cpp
  └── WindowManager.h       (application orchestrator)
        ├── StatsV1.h       (data collection)
        │     └── DataTypes.h
        ├── BarV1.h         (bar widget)
        ├── GaugeV1.h       (gauge widget)
        ├── ThemeV1.h       (color themes)
        ├── TileV1.h        (tile grid + layout config)
        ├── Rendering.h     (design system + stats data + tile renderers)
        │     ├── StatsV1.h
        │     ├── BarV1.h
        │     ├── GaugeV1.h
        │     ├── ThemeV1.h
        │     └── TileV1.h
        ├── LandingPage.h   (landing UI)
        │     └── ThemeV1.h
        ├── Overlays.h      (F2/F3 panels)
        │     ├── StatsV1.h
        │     └── ThemeV1.h
        ├── ConfigV1.h      (config persistence)
        └── LoggerV1.h      (CSV logging)

DataTypes.h (standalone — Win32/PDH types)
StatsV1.h → DataTypes.h
Tests/ → all src/ modules
```

## Working with PDH

### Counter path format
`\Category(Instance)\Counter`

### Rate counters need two collects
```cpp
PdhCollectQueryData(query);              // prime
// ... wait 200ms+ ...
PdhCollectQueryData(query);              // second collect
PdhGetFormattedCounterValue(counter, PDH_FMT_DOUBLE, nullptr, &val);
```

### Timing pattern (non-blocking)
All stats collectors use `QueryPerformanceCounter`:
1. First call: prime + record timestamp, return cached
2. Subsequent calls: check elapsed time via QPC
3. If < minimum interval: return cached
4. Otherwise: collect + compute + cache + return

### Adapter/GPU discovery
Uses `PdhExpandWildCardPathW` (more reliable across Windows locales than `PdhEnumObjectItemsW`):
```cpp
PdhExpandWildCardPathW(nullptr, L"\\Network Interface(*)\\Bytes Total/sec", nullptr, &size, 0);
```

### Network adapter classification
1. Get adapter names from expanded PDH wildcard paths
2. Wi-Fi: keyword match (wi-fi, wifi, wireless, wlan, 802.11)
3. Ethernet: non-WiFi that passes exclusion filters (bluetooth, virtual, loopback, teredo, isatap)
4. Fallback: single adapter → assign to ethernet

### GPU Multi-Instance Architecture
1. Expand `\GPU Engine(*)\Utilization Percentage` wildcard path → per-engine PDH queries
2. VRAM: `\GPU Adapter Memory(*)\Dedicated Usage` per-adapter PDH query
3. Total VRAM: DXGI adapter enumeration (`IDXGIFactory`, `IDXGIAdapter`, `DXGI_ADAPTER_DESC`)
4. Model: DXGI `DXGI_ADAPTER_DESC.Description` (more reliable than registry/EnumDisplayDevices)
5. Clock speed: WMI `Win32_VideoController` query for `CurrentClockSpeed`

## Thread Safety
- **StatsData** (Rendering.h): All members are `std::atomic<float/int>` — single-writer (updateStats thread), single-reader (renderLoop thread)
- **StatsV1**: All PDH state accessed ONLY from updateStats thread via GET* methods
- No locks, no mutexes — pure atomics for cross-thread sharing

## Constraints
- Windows 10+ only
- No third-party dependencies beyond CMakeLists.txt
- raylib 5.5 fetched via FetchContent at build time
- Do NOT commit build artifacts (*.exe, *.o, build/, .opencode/)
- Do NOT add Python scripts or requirements
- Keep files focused — each .h/.cpp pair handles one concern
