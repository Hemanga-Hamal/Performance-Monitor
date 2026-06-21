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
    │                       # Dashboard: tile grid rendering, status bar, CSV logging, tooltips
    ├── LandingPage.cpp/.h  # Landing page: theme picker (3 cards), "Start Monitoring" button
    │                       # Defines enum AppState { LANDING, DASHBOARD }
    ├── DiagnosticsOverlay.cpp/.h  # F2 diagnostics panel: frosted overlay, scroll, sections, Save
    ├── SettingsOverlay.cpp/.h     # F3 settings panel: tile/disk/adapter toggle checkboxes
    ├── StatsData.h         # Shared StatsData struct (atomics for CPU/RAM/GPU/Disk/Network)
    │                       # Used by both threads — single-writer, single-reader
    ├── StatsCollector.cpp / .h    # PDH + Win32 stats collector
    │                       # CPU: frequency (PDH), utilization (GetSystemTimes), model (registry)
    │                       # RAM: total/used/util (GlobalMemoryStatusEx)
    │                       # GPU: multi-instance via PdhExpandWildCardPathW("\\GPU Engine(*)\\...")
    │                       # Disk: all DRIVE_FIXED via GetLogicalDrives, per-disk enabled/disabled
    │                       # Network: WiFi/Ethernet via PdhExpandWildCardPathW, keyword filter
    │                       # Timing: non-blocking QPC-timed two-phase collection
    ├── BarWidget.cpp / .h      # Horizontal progress bar widget
    │                       # Rounded ends (DrawRectangleRounded), gradient fill, auto-scale or fixed mode
    │                       # Label truncation with "..." on narrow bars, hover tooltip
    │                       # getTotalHeight(), drawInRect(); supports Theme::fromAppTheme()
    ├── GaugeWidget.cpp / .h    # Circular arc gauge widget (300-segment triangle arc)
    │                       # Glow effect behind active arc, rounded endpoint cap (DrawCircleV)
    │                       # ConfigArc (240 deg), ConfigQuarter (270 deg) presets
    │                       # drawInRect(); supports Theme::fromAppTheme()
    ├── ConfigManager.cpp / .h   # Config persistence via INI file in %APPDATA%\PerfMon\config.ini
    │                       # Save/load: themeIndex, tileEnabled[5], window position/size
    ├── CsvLogger.cpp / .h   # CSV logger, F4 toggle, writes timestamped rows to working directory
    ├── TileItem.h            # Grid tile widget with drag-to-swap
    │                       # TileItem: col, row, spanCols, spanRows, title, bounds, computeBounds, titleBar
    │                       # GridLayout: cols, rows, offsetY, tiles vector, layout(), tileAt(), tileAtPos(),
    │                       #   swapPositions(), applyConfig()
    │                       # GridConfig/GridPresets: configurable grid sizes (4x6, 3x4, 5x6, 2x3, 3x6), F5 toggle
    │                       # Drag-to-swap: dragging a tile drops it onto another's grid position, swapping both
    │                       # Ghost preview: target tile glows with accent color during drag
    ├── AppTheme.h           # Color theme: Dark, Light, HighContrast static presets
    │                       # 22 color fields: windowBg, tileBg, tileBorder, tileShadow, titleText,
    │                       # textPrimary/Secondary/Muted, lineColor, barBackground/Foreground/ForegroundDim,
    │                       # gaugeArcBg/ArcActive/ArcGlow/GaugeText, landingBg, accentColor, accentHover,
    │                       # panelOverlay, toggleActive/Inactive, scrollbarColor
    ├── TitleBar.h / .cpp   # Custom 32px title bar (shared between LANDING and DASHBOARD states)
    │                       # Centered title, close button (X with hover color), window drag via GetMouseDelta
    ├── TitleBarWin32.cpp   # Win32 subclass: WM_NCHITTEST→HTCAPTION, DwmExtendFrameIntoClientArea
    └── tests/
        ├── test_main.cpp       # Test runner (63 tests, 7 groups)
        ├── test_harness.h      # Shared macros: TEST, CHECK, CHECK_EQ, CHECK_RANGE
        ├── test_cpu.cpp        # CPU tests (frequency, utilization, model name)
        ├── test_ram.cpp        # RAM tests (total, used, utilization)
        ├── test_gpu.cpp        # GPU tests (available, utilization, model, name)
        ├── test_disk.cpp       # Disk tests (count, name, total, used, utilization)
        ├── test_network.cpp    # Network tests (WiFi/Ethernet send/receive, adapter list)
        └── test_graphics.cpp   # Widget tests (BarWidget/GaugeWidget: theme, dims, config, sizing, presets)
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
- Classes: PascalCase (StatsCollector, BarWidget, GaugeWidget, TileItem, ConfigManager, CsvLogger)
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
| `StatsCollector` | StatsCollector.h | Stats collector (CPU/RAM/GPU/Disk/Network) |
| `StatsCollector::GPUInstance` | StatsCollector.h | Per-GPU PDH query, counter, name, displayName, cached utilization |
| `StatsCollector::DiskInfo` | StatsCollector.h | Per-disk info (name, total GB, used GB, utilization, enabled) |
| `StatsCollector::AdapterInfo` | StatsCollector.h | Per-adapter info (name, isWiFi, isEthernet, enabled) |
| `StatsCollector::NetworkCounters` | StatsCollector.h (private) | PDH handles + cached rates + timing for one interface |
| `BarWidget` | BarWidget.h | Horizontal progress bar widget |
| `BarWidget::Theme` | BarWidget.h | Bar colors; has `fromAppTheme()` factory accepting AppTheme |
| `BarWidget::Dimensions` | BarWidget.h | Bar sizing (width, height, scalingRatio, textSizeRatio, minSize, maxSize) |
| `BarWidget::Config` | BarWidget.h | Bar value config (value, maxValue, autoScale, screenSizeRatio) |
| `GaugeWidget` | GaugeWidget.h | Circular arc gauge widget |
| `GaugeWidget::Theme` | GaugeWidget.h | Gauge colors; has `fromAppTheme()` factory accepting AppTheme |
| `GaugeWidget::Dimensions` | GaugeWidget.h | Gauge sizing (baseSize, scaleRatio, arcThickness, textSizeRatio, minSize, maxSize) |
| `GaugeWidget::Config` | GaugeWidget.h | Gauge config (startAngle, totalAngle, autoScale, screenSizeRatio, method) |
| `TileItem` | TileItem.h | Grid tile struct (col, row, span, title, bounds) |
| `GridLayout` | TileItem.h | Grid layout manager with swap logic |
| `GridConfig` / `GridPresets` | TileItem.h | Configurable grid presets (4x6 default, 3x4, 5x6, 2x3, 3x6) |
| `AppTheme` | AppTheme.h | App-wide 22-field color theme (Dark, Light, HighContrast static presets) |
| `TitleBar` | TitleBar.h | Custom 32px title bar, handles drag + close |
| `StatsData` | StatsData.h | Atomic wrappers for cross-thread stats sharing |

## UI Architecture

### State machine
`enum AppState { LANDING, DASHBOARD }` (defined in LandingPage.h) — starts on landing page, transitions to dashboard on "Start Monitoring" click.

### Grid system (default: 4 columns × 6 rows)
```
Cell = screen / grid dimensions
Default tile layout:
  CPU:      (col=0, row=0, spanCols=2, spanRows=2)  — top-left
  RAM:      (col=2, row=0, spanCols=2, spanRows=2)  — top-right
  GPU:      (col=0, row=2, spanCols=2, spanRows=2)  — mid-left
  Network:  (col=2, row=2, spanCols=2, spanRows=2)  — mid-right
  Storage:  (col=0, row=4, spanCols=4, spanRows=2)  — bottom-full-width
Total tiles: 5. Tile area: screen height - 30px status bar - 32px custom title bar.
Tile bounds use `boundsYOffset` (set to TITLE_BAR_H=32) to shift grid into screen coordinates.
Custom title bar drawn at top of undecorated window (FLAG_WINDOW_UNDECORATED).
```

### Grid presets (F5 toggle)
| Preset | Cols × Rows |
|--------|-------------|
| Default | 4 × 6 |
| Compact | 3 × 4 |
| Wide    | 5 × 6 |
| Minimal | 2 × 3 |
| Tall    | 3 × 6 |

### Tile system
- `computeBounds()` — initial grid layout (skipped if already positioned and screen unchanged). Adds `boundsYOffset` to Y.
- `handleDrag()` — mouse-driven drag (title bar) and resize (bottom-right grip). Off-screen clamping respects `boundsYOffset`.
- `computeSnapPreview()` — ghost preview computed from tile top-left edge (not center) so large tiles can reach row 0. Includes `boundsYOffset`.
- `snapToGrid()` — on release: snaps position + auto-computes spanCols/spanRows from bounds/cell ratio. Includes `boundsYOffset` in target.
- `resolveCollisions()` — scans ALL grid cells (0..gridRows × 0..gridCols) for empty space for displaced tiles. Respects per-tile `boundsYOffset`.
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

### Design system
- **AppTheme** is the canonical 22-field color theme. The Dark, Light, and HighContrast presets are static factory methods on AppTheme.
- **BarWidget** and **GaugeWidget** each have their own `Theme` struct for backward compatibility. To create a widget theme from the app theme, use the static factory:
  - `BarWidget::Theme::fromAppTheme(activeTheme)` — maps barBackground, barForeground, barForegroundDim, textPrimary
  - `GaugeWidget::Theme::fromAppTheme(activeTheme)` — maps tileBg, gaugeArcBg, gaugeArcActive, gaugeText

### Overlays
- **F2 Diagnostics:** `DiagnosticsOverlay()` — Centered frosted panel (80% × 80% screen), scissor-clipped scroll region, accent-colored section headers with line dividers, scrollbar, "Save" pill button.
- **F3 Settings:** `SettingsOverlay()` — Right-aligned frosted panel (370px wide, 80% screen height), scissor-clipped scroll, rounded checkbox toggles with checkmarks, per-tile / per-disk / per-adapter sections with section headers.
- **F4 CSV Logging:** Toggle on/off. Status shown in key-hint bar at window bottom. Writes to `perfmon_YYYYMMDD_HHMMSS.csv` in working directory.

### Landing page
`DrawLandingPage()` — Centered hero title + subtitle, 3 theme cards (Dark, Light, High Contrast) with drop shadows, hover glow, selected state with accent border, color swatch previews, "Start Monitoring" pill button. Returns true when button clicked. Layout scales with `cardScale = clamp(min(sw/1300, sh/850), 0.5, 2.0)`.

### Status bar
- 30px bar at `y = screenH - 30` with `windowBg` background and separator line
- Left: "F2:Diag  F3:Settings  F4:Log" in muted text
- Right: "F4:Log [ON/OFF]" in accent color (ON) or muted (OFF)
- FPS counter at top-right

### Custom title bar
- 32px themed bar at top (replaces default raylib window bar via FLAG_WINDOW_UNDECORATED)
- Centered title "Performance Monitor", close button (X) on right, window drag via mouse delta
- Present on both landing page and dashboard
- Win32 subclass in TitleBarWin32.cpp handles WM_NCHITTEST→HTCAPTION for native window drag

## Module API Signatures

```cpp
// LandingPage.h — also defines enum AppState { LANDING, DASHBOARD }
bool DrawLandingPage(int sw, int sh, int titleSize, int fontSize,
                     const AppTheme& theme, int& selectedThemeIndex,
                     AppTheme& activeTheme, AppState& appState);

// DiagnosticsOverlay.h
void DrawDiagnosticsOverlay(int sw, int sh, int fontSize,
                            const AppTheme& theme,
                            StatsData& statsData, StatsCollector& stats);

// SettingsOverlay.h
void DrawSettingsOverlay(int sw, int sh, int fontSize,
                         const AppTheme& theme,
                         bool tileEnabled[], StatsCollector& stats);
```

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
5. Display as "GPU 1", "GPU 2" bars

## Thread Safety
- **StatsData** (StatsData.h): All members are `std::atomic<float/int>` — single-writer (updateStats thread), single-reader (renderLoop thread)
- **StatsCollector**: All PDH state accessed ONLY from updateStats thread via GET* methods
- No locks, no mutexes — pure atomics for cross-thread sharing

## Constraints
- Windows 10+ only
- No third-party dependencies beyond CMakeLists.txt
- raylib 5.5 fetched via FetchContent at build time
- Do NOT commit build artifacts (*.exe, *.o, build/, .opencode/)
- Do NOT add Python scripts or requirements
- Keep files focused — each .h/.cpp pair handles one concern
