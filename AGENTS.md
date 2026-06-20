# AI Agent Guide — Performance Monitor

## Overview
Windows-only C++17 desktop app that displays real-time system performance metrics using raylib 5.5 for rendering and the Windows PDH (Performance Data Helper) API for data collection. Features a 4x6 grid-based tile dashboard with drag-to-arrange, 3 themes, config persistence, CSV logging, and a 63-test suite.

## Tech Stack
- **Language:** C++17
- **Graphics:** raylib 5.5 (fetched via CMake FetchContent)
- **System APIs:** PDH (pdh.lib), Win32 (kernel32), Winsock (ws2_32), Shell32 (SHGetFolderPathW for config path)
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
    ├── main.cpp            # Entry point, 2 threads (updateStats + renderLoop)
    │                       # Landing page: theme picker with 3 cards, "Start Monitoring" button
    │                       # Dashboard: 5 tiles in 4x6 grid, F2 diagnostics, F3 settings, F4 CSV logging
    │                       # Status bar: 30px bar at bottom with key hints + log status
    │                       # Tile content: per-tile gauge/bar rendering with content-boundary clamping
    ├── StatsV1.cpp / .h    # PDH + Win32 stats collector
    │                       # CPU: frequency (PDH), utilization (GetSystemTimes), model (registry)
    │                       # RAM: total/used/util (GlobalMemoryStatusEx)
    │                       # GPU: multi-instance via PdhExpandWildCardPathW("\\GPU Engine(*)\\...")
    │                       # Disk: all DRIVE_FIXED via GetLogicalDrives, per-disk enabled/disabled
    │                       # Network: WiFi/Ethernet via PdhExpandWildCardPathW, keyword filter
    │                       # Timing: non-blocking QPC-timed two-phase collection
    ├── BarV1.cpp / .h      # Horizontal progress bar widget
    │                       # Rounded ends (DrawRectangleRounded), auto-scale or fixed mode
    │                       # Label truncation with "..." on narrow bars
    │                       # getTotalHeight() returns bar+text full vertical space
    │                       # drawInRect() for bounded rendering within rectangles
    ├── GaugeV1.cpp / .h    # Circular arc gauge widget (300-segment triangle arc)
    │                       # Glow effect behind active arc, rounded endpoint cap (DrawCircleV)
    │                       # ConfigArc (240 deg), ConfigQuarter (270 deg) presets
    │                       # drawInRect() for bounded rendering within rectangles
    ├── ConfigV1.cpp / .h   # Config persistence via INI file in %APPDATA%\PerfMon\config.ini
    │                       # Save/load: themeIndex, tileEnabled[5], window position/size
    ├── LoggerV1.cpp / .h   # CSV logger, F4 toggle, writes timestamped rows to working directory
    ├── TileV1.h            # Grid tile widget (4x6 default grid)
    │                       # computeBounds (grid layout), handleDrag (mouse drag/resize),
    │                       # computeSnapPreview (ghost during drag), snapToGrid (on release),
    │                       # resolveCollisions (scan all grid cells for empty space),
    │                       # drawFrame (shadow, rounded border, title, separator, grip),
    │                       # Auto-span: spanCols/spanRows recomputed from bounds/cell ratio on snap
    ├── ThemeV1.h           # Color theme: Dark, Light, HighContrast presets
    │                       # 22 color fields: window, tile, shadow, accent, hover, panel, etc.
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
- Classes: PascalCase (StatsV1, BarV1, GaugeV1, TileV1, ConfigV1, LoggerV1)
- Methods: camelCase (setValue, calculateBarSize, getTotalHeight)
- PDH getters: ALLCAPS (GETCPUFrequency, GETRAMUsed)
- Members: camelCase (cpuQuery, barWidth, gpuInstances, adapterInfos)
- Structs: PascalCase nested inside classes (Theme, Dimensions, Config, GPUInstance, DiskInfo, AdapterInfo)

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
| `StatsV1::GPUInstance` | StatsV1.h | Per-GPU PDH query, counter, name, displayName, cached utilization |
| `StatsV1::DiskInfo` | StatsV1.h | Per-disk info (name, total GB, used GB, utilization, enabled) |
| `StatsV1::AdapterInfo` | StatsV1.h | Per-adapter info (name, isWiFi, isEthernet, enabled) |
| `StatsV1::NetworkCounters` | StatsV1.h (private) | PDH handles + cached rates + timing for one interface |
| `BarV1` | BarV1.h | Horizontal progress bar widget |
| `BarV1::Theme` | BarV1.h | Bar colors (background, foreground, text) |
| `BarV1::Dimensions` | BarV1.h | Bar sizing (width, height, scalingRatio, textSizeRatio, minSize, maxSize) |
| `BarV1::Config` | BarV1.h | Bar value config (value, maxValue, autoScale, screenSizeRatio) |
| `GaugeV1` | GaugeV1.h | Circular arc gauge widget |
| `GaugeV1::Theme` | GaugeV1.h | Gauge colors (bg, arcBg, arcActive, text) |
| `GaugeV1::Dimensions` | GaugeV1.h | Gauge sizing (baseSize, scaleRatio, arcThickness, textSizeRatio, minSize, maxSize) |
| `GaugeV1::Config` | GaugeV1.h | Gauge config (startAngle, totalAngle, autoScale, screenSizeRatio, method) |
| `TileV1` | TileV1.h | Tile widget (bounds, drag, resize, snap, collision avoidance) |
| `TileConfig` | TileV1.h | Tile grid position (col, row, spanCols, spanRows, title) |
| `ThemeV1` | ThemeV1.h | App-wide 22-field color theme (Dark, Light, HighContrast static presets) |
| `AppConfig` | ConfigV1.h | Persisted config (themeIndex, tileEnabled[5], window pos/size) |
| `ConfigV1` | ConfigV1.h | Load/save AppConfig from %APPDATA%\PerfMon\config.ini |
| `LoggerV1` | LoggerV1.h | CSV log writer (F4 toggle, timestamped rows) |
| `StatsData` | main.cpp | Atomic wrappers for cross-thread stats sharing |

## UI Architecture

### State machine
`enum AppState { LANDING, DASHBOARD }` — starts on landing page, transitions to dashboard on "Start Monitoring" click.

### Grid system (current: 4 columns × 6 rows)
```
Cell = screen / grid dimensions
Default tile layout:
  CPU:      (col=0, row=0, spanCols=2, spanRows=2)  — top-left
  RAM:      (col=2, row=0, spanCols=2, spanRows=2)  — top-right
  GPU:      (col=0, row=2, spanCols=2, spanRows=2)  — mid-left
  Network:  (col=2, row=2, spanCols=2, spanRows=2)  — mid-right
  Storage:  (col=0, row=4, spanCols=4, spanRows=2)  — bottom-full-width
Total tiles: 5. Tile area height: screen height - 30px status bar.
```

### Tile system
- `computeBounds()` — initial grid layout (skipped if already positioned and screen unchanged)
- `handleDrag()` — mouse-driven drag (title bar) and resize (bottom-right grip). Off-screen clamping.
- `computeSnapPreview()` — ghost preview computed from tile top-left edge (not center) so large tiles can reach row 0
- `snapToGrid()` — on release: snaps position + auto-computes spanCols/spanRows from bounds/cell ratio
- `resolveCollisions()` — scans ALL grid cells (0..gridRows × 0..gridCols) for empty space for displaced tiles
- `drawFrame()` — drop shadow (offset 3x3), rounded rect (0.06f), title text (auto-shrink to fit), separator line, resize grip triangle

### Content per tile
- **CPU:** Model text (auto-shrink to fit width) → gauge (arc, 240°) → bar (frequency)
- **RAM:** Gauge only (quarter, 270°)
- **GPU:** Model text → N bars (one per GPU instance, labeled "GPU 1", "GPU 2", etc.)
- **Network:** 4 bars (WiFi Up/Down, Eth Up/Down) with compact labels
- **Storage:** N bars (one per enabled disk)

### Content sizing
- All content positioned relative to `titleBar()` bottom and `bounds` edges, NOT the global window
- Title font: `min(bounds.w * 0.08, bounds.h * 0.09, 24)` — tile-relative
- Model text font: `min(bounds.w * 0.04, 16)` — tile-relative
- Bar maxSize: `contentWidth * 0.78-0.88` — tile-relative
- Gauge size: `min(contentWidth * 0.48f, contentH * 0.xx)` — tile-relative
- Spacing between bars uses `bar.getTotalHeight()` (bar rectangle + label text height)

### Overlays
- **F2 Diagnostics:** Centered frosted panel (80% × 80% screen), scissor-clipped scroll region, accent-colored section headers with line dividers, elegantly-styled scrollbar, "Save" pill button.
- **F3 Settings:** Right-aligned frosted panel (370px wide, 80% screen height), scissor-clipped scroll, rounded checkbox toggles with checkmarks, per-tile / per-disk / per-adapter sections with section headers.
- **F4 CSV Logging:** Toggle on/off. Status shown in key-hint bar at window bottom. Writes to `perfmon_YYYYMMDD_HHMMSS.csv` in working directory.

### Status bar
- 30px bar at `y = screenH - 30` with `windowBg` background and separator line
- Left: "F2:Diag  F3:Settings  F4:Log" in muted text
- Right: "F4:Log [ON/OFF]" in accent color (ON) or muted (OFF)
- FPS counter at top-right

### Landing page
- Centered hero title + subtitle
- 3 theme cards (Dark, Light, High Contrast) with drop shadows, hover glow, selected state with accent border
- Color swatch previews (rounded rectangles)
- "Start Monitoring" pill button with shadow + hover highlight
- Full layout scales with `cardScale = clamp(min(sw/1300, sh/850), 0.5, 2.0)`

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
All stats collectors use `QueryPerformanceCounter` for timing:
1. First call: prime + record timestamp, return cached
2. Subsequent calls: check elapsed time via QPC
3. If < minimum interval: return cached
4. Otherwise: collect + compute + cache + return

### Adapter/GPU discovery (current approach)
Uses `PdhExpandWildCardPathW` instead of `PdhEnumObjectItemsW`:
```cpp
PdhExpandWildCardPathW(nullptr, L"\\Network Interface(*)\\Bytes Total/sec", nullptr, &size, 0);
// Extracts instance names from expanded paths like "\Network Interface(Intel[R]...)\Bytes Total/sec"
```
This is more reliable across Windows locales than `PdhEnumObjectItemsW`.

### Network adapter classification
1. Get all adapter names from expanded PDH wildcard paths
2. Wi-Fi detection: keyword match (wi-fi, wifi, wireless, wlan, 802.11)
3. Ethernet: any non-WiFi adapter that passes exclusion filters (bluetooth, virtual, loopback, teredo, isatap)
4. Fallback: single adapter → assign to ethernet

### GPU Multi-Instance Architecture
1. Expand `\GPU Engine(*)\Utilization Percentage` wildcard path
2. Parse instance names from expanded paths (skip `_Total`)
3. Create separate PDH query per GPU instance
4. Store in `vector<GPUInstance>` with per-instance cached utilization
5. Display as "GPU 1", "GPU 2" bars (displayName mapping exists but not yet wired to public API)

## Thread Safety
- **StatsData** (main.cpp): All members are `std::atomic<float/int>` — single-writer (updateStats thread), single-reader (renderLoop thread)
- **StatsV1**: All PDH state accessed ONLY from updateStats thread via GET* methods
- No locks, no mutexes — pure atomics for cross-thread sharing

## Constraints
- Windows 10+ only
- No third-party dependencies beyond CMakeLists.txt
- raylib 5.5 fetched via FetchContent at build time
- Do NOT commit build artifacts (*.exe, *.o, build/, .opencode/)
- Do NOT add Python scripts or requirements
- Keep files focused — each .h/.cpp pair handles one concern
