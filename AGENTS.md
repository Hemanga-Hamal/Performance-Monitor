# AI Agent Guide â€” Performance Monitor

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
â”œâ”€â”€ CMakeLists.txt          # FetchContent raylib 5.5, targets: perfmon.exe + perfmon_tests.exe
â”œâ”€â”€ .gitignore              # Ignores build/, *.exe, *.o, IDE files
â”œâ”€â”€ PLAN.md                 # High-level project roadmap + phase history
â”œâ”€â”€ TODO.md                 # Known issues + future tasks for AI agents
â”œâ”€â”€ README.md               # User-facing build/usage docs
â”œâ”€â”€ resources/
â”‚   â””â”€â”€ LICENSE             # Project license
â””â”€â”€ src/
    â”œâ”€â”€ main.cpp            # Entry point (~10 lines): just calls WindowManager::run()
    â”œâ”€â”€ WindowManager.h     # Application orchestrator: global state, config load/save, updateStats thread,
    â”‚                       # renderLoop (state machine, F2/F3/F4 dispatch, status bar, config save on exit)
    â”œâ”€â”€ Rendering.h         # Unified rendering module:
    â”‚                       #   DesignSystem namespace: tileTitleFont, modelFont, makeBarTheme, makeGaugeTheme
    â”‚                       #   StatsData struct (atomic cross-thread stats)
    â”‚                       #   formatValue helper
    â”‚                       #   renderCPUTile, renderRAMTile, renderGPUTile, renderNetworkTile, renderStorageTile
    â”œâ”€â”€ LandingPage.h       # Landing page UI: AppState enum, drawLandingPage (hero, 3 theme cards, Start button)
    â”œâ”€â”€ Overlays.h          # F2 diagnostics overlay, F3 settings overlay (scrollable frosted panels)
    â”œâ”€â”€ Tile.h            # Tile grid system: TileConfig, LayoutConfig (4x4 grid constants),
    â”‚                       # Tile class (cell-based occupancy, drag/resize, displacement cascade,
    â”‚                       # ghost preview, snap-on-release, drawFrame, createDefaultTiles)
    â”œâ”€â”€ Theme.h           # Color theme: Dark, Light, HighContrast presets â€” 22 color fields
    â”œâ”€â”€ DataTypes.h         # Shared type definitions: DiskInfo, AdapterInfo, GPUInstance
    â”œâ”€â”€ Stats.cpp / .h    # PDH + DXGI + WMI stats collector
    â”‚                       # CPU: frequency (PDH), utilization (GetSystemTimes), model (registry)
    â”‚                       # RAM: total/used/util (GlobalMemoryStatusEx)
    â”‚                       # GPU: utilization (WMI per-LUID engine), VRAM (PDH dedicated matched by physical index),
    â”‚                       #   total VRAM (DXGI), model (DXGI adapter desc), clock speed (WMI Win32_VideoController)
    â”‚                       # Disk: all DRIVE_FIXED via GetLogicalDrives, per-disk enabled/disabled
    â”‚                       # Network: WiFi/Ethernet via PdhExpandWildCardPathW, keyword filter
    â”‚                       # Timing: non-blocking QPC-timed two-phase collection
    â”œâ”€â”€ Bar.cpp / .h      # Horizontal progress bar widget
    â”œâ”€â”€ Gauge.cpp / .h    # Circular arc gauge widget (300-segment triangle arc)
    â”œâ”€â”€ Config.cpp / .h   # Config persistence via INI file in %APPDATA%\PerfMon\config.ini
    â”œâ”€â”€ Logger.cpp / .h   # CSV logger, F4 toggle, writes timestamped rows to working directory
    â””â”€â”€ tests/
        â”œâ”€â”€ test_main.cpp       # Test runner (63 tests, 7 groups)
        â”œâ”€â”€ test_harness.h      # Shared macros: TEST, CHECK, CHECK_EQ, CHECK_RANGE
        â”œâ”€â”€ test_cpu.cpp        # CPU tests (frequency, utilization, model name)
        â”œâ”€â”€ test_ram.cpp        # RAM tests (total, used, utilization)
        â”œâ”€â”€ test_gpu.cpp        # GPU tests (available, utilization, model, name)
        â”œâ”€â”€ test_disk.cpp       # Disk tests (count, name, total, used, utilization)
        â”œâ”€â”€ test_network.cpp    # Network tests (WiFi/Ethernet send/receive, adapter list)
        â””â”€â”€ test_graphics.cpp   # Widget tests (Bar: theme/dims/config/sizing; Gauge: clamp/presets/setters)
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
- **Include guards:** `#ifndef HEADER_H` / `#define HEADER_H` / `#endif` â€” NOT `#pragma once`
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
- Classes: PascalCase (Stats, Bar, Gauge, Tile, Config, Logger, WindowManager)
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
- Win32 failures: same pattern â€” return 0.0f or empty string
- Do NOT add logging, exceptions, or error messages unless explicitly asked

## Key Types

| Type | Location | Purpose |
|------|----------|---------|
| `WindowManager` | WindowManager.h | Application orchestrator: state, threads, main loop |
| `Stats` | Stats.h | Stats collector (CPU/RAM/GPU/Disk/Network) |
| `GPUInstance` | DataTypes.h | Per-GPU PDH query + counter handles, VRAM query, name, displayName, cached values |
| `DiskInfo` | DataTypes.h | Per-disk info (name, total GB, used GB, utilization, enabled) |
| `AdapterInfo` | DataTypes.h | Per-adapter info (name, isWiFi, isEthernet, enabled) |
| `StatsData` | Rendering.h | Atomic wrappers for cross-thread stats sharing (CPU, RAM, GPU util/VRAM/clock, network, disk) |
| `Bar` | Bar.h | Horizontal progress bar widget |
| `Bar::Theme` | Bar.h | Bar colors (background, foreground, text) |
| `Bar::Dimensions` | Bar.h | Bar sizing (width, height, scalingRatio, textSizeRatio, minSize, maxSize) |
| `Bar::Config` | Bar.h | Bar value config (value, maxValue, autoScale, screenSizeRatio) |
| `Gauge` | Gauge.h | Circular arc gauge widget |
| `Gauge::Theme` | Gauge.h | Gauge colors (bg, arcBg, arcActive, text) |
| `Gauge::Dimensions` | Gauge.h | Gauge sizing (baseSize, scaleRatio, arcThickness, textSizeRatio, minSize, maxSize) |
| `Gauge::Config` | Gauge.h | Gauge config: ConfigArc (240 deg), ConfigQuarter (270 deg) |
| `Tile` | Tile.h | Tile widget: cell-based occupancy grid, drag/resize, displacement cascade |
| `TileConfig` | Tile.h | Tile grid position (col, row, spanCols, spanRows, minSpanCols, minSpanRows, title) |
| `LayoutConfig` | Tile.h | Grid constants: gridCols=4, gridRows=4, statusBarH=30, tilePadding=10, maxTiles=5 |
| `Theme` | Theme.h | App-wide 22-field color theme (Dark, Light, HighContrast static presets) |
| `AppConfig` | Config.h | Persisted config (themeIndex, tileEnabled[5], window pos/size) |
| `Config` | Config.h | Load/save AppConfig from %APPDATA%\PerfMon\config.ini |
| `Logger` | Logger.h | CSV log writer (F4 toggle, timestamped rows) |
| `AppState` | LandingPage.h | State machine enum: LANDING, DASHBOARD |

## UI Architecture

### State machine
`enum AppState { LANDING, DASHBOARD }` â€” starts on landing page, transitions to dashboard on "Start Monitoring" click.

### Grid system (current: 4 columns Ã— 4 rows)
```
Cell = screen / grid dimensions
Default tile layout:
  CPU:      (col=0, row=0, spanCols=2, spanRows=2)  â€” top-left
  RAM:      (col=0, row=2, spanCols=2, spanRows=2)  â€” bottom-left
  GPU:      (col=2, row=0, spanCols=2, spanRows=2)  â€” top-right
  Network:  (col=2, row=2, spanCols=2, spanRows=2)  â€” bottom-right
  Storage:  (col=3, row=0, spanCols=4, spanRows=1)  â€” full-width narrow row (takes remaining row)
Total tiles: 5. Tile area height: screen height - 30px status bar.
```
Grid config is in `LayoutConfig` struct inside `Tile.h`.

### Tile system (cell-based displacement algorithm)
1. **Occupancy grid:** `buildOccupancy()` builds a bool[4][4] from all tiles' config positions
2. **Fit check:** `fitsInGrid()` tests if a config fits at (row,col) without overlap
3. **Displacement cascade:** `tryPlaceTile()` finds conflicting tiles, unmarks them, places the dragged tile first, then calls `tryShiftTile()` on each displaced tile to find the best alternative position (minimizing distance from original)
4. **Ghost preview:** `updateGhostPreview()` shows the dragged tile's projected position (accent ghost fill) plus all displaced tiles' new positions (muted ghost fill)
5. **Rejection:** If displacement fails (any displaced tile can't find a valid cell), the drag snaps back to original position
6. **Min-size enforcement:** Each tile has `minSpanCols`/`minSpanRows` â€” a tile-sized opening in the grid that violates these minimums is rejected

### Content per tile
- **CPU:** Model text (auto-shrink to fit width) â†’ gauge (arc, 240Â°) â†’ bar (frequency)
- **RAM:** Gauge (quarter, 270Â°) â†’ bar (used / total GB)
- **GPU:** Gauge (arc, 240Â°, utilization %) â†’ VRAM bar (used/total GB) â†’ Clock bar (MHz, 0-100 scale, max 3000 MHz)
- **Network:** 4 bars (WiFi Up/Down, Eth Up/Down) with compact labels
- **Storage:** N bars (one per enabled disk)

Per-tile rendering is in `Rendering.h`.

### Content sizing
- All content positioned relative to `tile.bounds` (not the global window)
- Title font: `DesignSystem::tileTitleFont(tileW, tileH)` â€” tile-relative
- Model text font: `DesignSystem::modelFont(tileW)` â€” tile-relative
- Widget themes mapped from `Theme` via `DesignSystem::makeBarTheme()` / `makeGaugeTheme()`

### Overlays
- **F2 Diagnostics:** Centered frosted panel (80% Ã— 80% screen), scissor-clipped scroll region, accent-colored section headers, scrollbar, "Save" exports diagnostics.txt
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
  â””â”€â”€ WindowManager.h       (application orchestrator)
        â”œâ”€â”€ Stats.h       (data collection)
        â”‚     â””â”€â”€ DataTypes.h
        â”œâ”€â”€ Bar.h         (bar widget)
        â”œâ”€â”€ Gauge.h       (gauge widget)
        â”œâ”€â”€ Theme.h       (color themes)
        â”œâ”€â”€ Tile.h        (tile grid + layout config)
        â”œâ”€â”€ Rendering.h     (design system + stats data + tile renderers)
        â”‚     â”œâ”€â”€ Stats.h
        â”‚     â”œâ”€â”€ Bar.h
        â”‚     â”œâ”€â”€ Gauge.h
        â”‚     â”œâ”€â”€ Theme.h
        â”‚     â””â”€â”€ Tile.h
        â”œâ”€â”€ LandingPage.h   (landing UI)
        â”‚     â””â”€â”€ Theme.h
        â”œâ”€â”€ Overlays.h      (F2/F3 panels)
        â”‚     â”œâ”€â”€ Stats.h
        â”‚     â””â”€â”€ Theme.h
        â”œâ”€â”€ Config.h      (config persistence)
        â””â”€â”€ Logger.h      (CSV logging)

DataTypes.h (standalone â€” Win32/PDH types)
Stats.h â†’ DataTypes.h
Tests/ â†’ all src/ modules
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
4. Fallback: single adapter â†’ assign to ethernet

### GPU Multi-Instance Architecture
1. DXGI adapter enumeration for model name + total VRAM + LUID
2. PDH `\GPU Adapter Memory(*)\Dedicated Usage` for VRAM usage, matched to GPU by physical adapter index (attached BEFORE sort)
3. WMI `Win32_PerfFormattedData_GPUPerformanceCounters_GPUEngine` for per-engine utilization, aggregated by LUID
4. WMI `Win32_VideoController` clock speed
5. Persistent WMI connection (`IWbemLocator` + `IWbemServices`) for periodic GPU utilization queries
6. Sort: GPUs sorted by `vramTotalGB` descending (dedicated GPUs first) â€” PDH VRAM counters attached BEFORE sort to avoid cross-wiring

## Thread Safety
- **StatsData** (Rendering.h): All members are `std::atomic<float/int>` â€” single-writer (updateStats thread), single-reader (renderLoop thread)
- **Stats**: All PDH state accessed ONLY from updateStats thread via GET* methods
- No locks, no mutexes â€” pure atomics for cross-thread sharing

## Constraints
- Windows 10+ only
- No third-party dependencies beyond CMakeLists.txt
- raylib 5.5 fetched via FetchContent at build time
- Do NOT commit build artifacts (*.exe, *.o, build/, .opencode/)
- Do NOT add Python scripts or requirements
- Keep files focused â€” each .h/.cpp pair handles one concern
