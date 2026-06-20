# Project Plan: Performance Monitor

## Status: Production-Ready — All Phases Complete

## Overview
Windows-only C++17 performance monitor using raylib 5.5 and the PDH API. Monitors CPU frequency/utilization, RAM, multi-GPU utilization, disk usage, and network throughput in real time. Features a tile-based dashboard UI with drag-to-arrange tiles, theme support, device diagnostics, config persistence, CSV logging, and a 63-test suite.

## Build
```powershell
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
# Output: build\Release\perfmon.exe
# Tests: build\Release\perfmon_tests.exe  (63 pass, 0 fail)
```

## File Map

```
src/
├── main.cpp              # Entry point, landing page, tile dashboard, F2 diagnostics,
│                         # F3 settings (per-disk/adapter toggles), F4 CSV logging, status bar
├── StatsV1.cpp / .h      # PDH stats collector: CPU freq/util (PDH + GetSystemTimes),
│                         # RAM used/util (GlobalMemoryStatusEx), multi-GPU per-instance
│                         # (PdhExpandWildCardPathW), multi-disk enumeration (GetLogicalDrives),
│                         # network adapter discovery (PdhExpandWildCardPathW + keyword filter),
│                         # per-disk/per-adapter enable/disable, GPU displayName mapping
├── BarV1.cpp / .h        # Horizontal progress bar: rounded ends (DrawRectangleRounded),
│                         # auto-scale or fixed sizing, label truncation with "...",
│                         # getTotalHeight() for spacing, drawInRect() for bounded rendering
├── GaugeV1.cpp / .h      # Circular arc gauge: 300-segment triangle arc, glow behind active arc,
│                         # rounded endpoint cap (DrawCircleV), auto-scale or fixed sizing,
│                         # ConfigArc (240°), ConfigQuarter (270°) presets, drawInRect()
├── ConfigV1.cpp / .h     # Config persistence: saves/loads theme, tile state, window pos/size
│                         # to %APPDATA%\PerfMon\config.ini (INI format)
├── LoggerV1.cpp / .h     # CSV logger: F4 toggle, writes timestamped metrics rows
│                         # to perfmon_YYYYMMDD_HHMMSS.csv in working directory
├── TileV1.h              # Grid tile widget: computeBounds (grid layout), handleDrag (mouse),
│                         # computeSnapPreview (ghost from top-left edge), snapToGrid (on release),
│                         # resolveCollisions (scan 0..gridRows × 0..gridCols for empty cells),
│                         # drawFrame (shadow, rounded rect, title, separator, grip),
│                         # auto-span: spanCols/spanRows recomputed from bounds/cell ratio on snap
├── ThemeV1.h             # Color theme: Dark, Light, HighContrast static presets
│                         # 22 fields: windowBg, tileBg, tileBorder, tileShadow, titleText,
│                         # textPrimary/Secondary/Muted, lineColor, barBackground/Foreground/ForegroundDim,
│                         # gaugeArcBg/ArcActive/ArcGlow/GaugeText, landingBg, accentColor, accentHover,
│                         # panelOverlay, toggleActive/Inactive, scrollbarColor
└── tests/
    ├── test_main.cpp       # Test runner (calls all 7 groups)
    ├── test_harness.h      # Shared macros: TEST, CHECK, CHECK_EQ, CHECK_RANGE
    ├── test_cpu.cpp        # CPUTests: frequency, utilization, model name (5 tests)
    ├── test_ram.cpp        # RAMTests: total, used, utilization (4 tests)
    ├── test_gpu.cpp        # GPUTests: available, utilization, model, name (4 tests)
    ├── test_disk.cpp       # DiskTests: count, per-disk name/total/used/util (15 tests)
    ├── test_network.cpp    # NetworkTests: WiFi/Ethernet send/receive, adapter list (5 tests)
    └── test_graphics.cpp   # GraphicsTests: BarV1 theme/dims/config/sizing (12 tests),
                            # GaugeV1 clamp/presets/setters (18 tests)
```

## Phase History

### Phase 0: Scaffolding — DONE
- AGENTS.md, .gitignore, PLAN.md created

### Phase 1: Structural Consolidation — DONE
- Dead files removed: CPU_FQ-v0.cpp, CPU_FQ-v1.cpp, Graphicv2.cpp, PDH FIXEDD.txt
- Graphicv2T.cpp renamed to main.cpp
- CMakeLists.txt created with FetchContent for raylib 5.5
- Header/source separation for all components

### Phase 2: Robustness — DONE
- All Sleep() calls eliminated (non-blocking QPC-timed PDH collection)
- Dead code in GETCPUFrequency fixed (merged guard)
- Dynamic network adapter discovery (first PdhEnumObjectItemsW, later PdhExpandWildCardPathW)
- Network swprintf_s bug fixed (adapter name was replaced by direction string)
- Thread safety: atomic cache values, single-writer pattern

### Phase 3: Build & Distribution — DONE
- CMakeLists.txt targets: perfmon.exe + perfmon_tests.exe
- MSVC #pragma removed, linking in CMake target_link_libraries
- Unused includes removed
- GaugeV1 missing setters implemented
- README.md updated

### Phase 4: GPU Monitoring — DONE
- PDH GPU Engine query (cross-vendor: `\GPU Engine(*)\Utilization Percentage`)
- Non-blocking two-phase pattern (same as CPU)
- GPU model name via EnumDisplayDevices
- CPU model name via registry (HKLM\HARDWARE\DESCRIPTION\...)

### Phase 5: Tile UI — DONE
- TileV1 widget: computeBounds, handleDrag, snapToGrid, drawFrame, snapPreview
- Drag title bar to move, drag corner grip to resize
- Snap-to-grid on release with ghost preview during drag
- Collision avoidance: resolveCollisions scans all grid cells for empty space
- Auto-span: spanCols/spanRows recomputed from bounds/cell ratio on snap

### Phase 6: Overlays — DONE
- F2: Scrollable diagnostics with scissor clipping, section headers with line dividers, Save button
- F3: Device visibility toggles with per-disk/per-adapter sections
- Landing page: theme selector (3 cards with color swatches), Start button

### Phase 7: Testing — DONE
- 63 tests, 7 groups, 0 failures

### Phase 8: Feature Expansion — DONE
- Multi-GPU per-instance monitoring using PdhExpandWildCardPathW
- Per-disk and per-adapter enable/disable in F3 settings
- BarV1::drawInRect and GaugeV1::drawInRect for bounded rendering
- Config file persistence (INI in %APPDATA%\PerfMon\config.ini)
- LoggerV1: CSV logging with F4 toggle, timestamped metrics rows
- Export diagnostics to text file (Save button in F2 overlay)
- Config persistence: theme, tileEnabled[5], window position/size

### Phase 9: UI Polish — DONE
- Migrated from 2x4 to 4x6 grid for finer snapping
- Drop shadows on tiles and overlays
- BarV1 rounded ends (DrawRectangleRounded)
- GaugeV1 glow effect + endpoint cap
- Frosted glass overlay panels (semi-transparent bg + border)
- Rounded checkbox toggles with checkmarks in F3
- Status bar with background + separator at window bottom
- Text auto-shrinking to fit tile width (model names, bar labels)
- Font sizes computed from tile dimensions, not window dimensions
- Network labels shortened to "WiFi Up/Down", "Eth Up/Down"
- GPU labels simplified to "GPU 1", "GPU 2" for multi-GPU
- Collision avoidance scans ALL rows (including above) for displaced tiles
- Snap preview computed from top-left edge so large tiles can reach row 0

### Phase 10: Bug Fixes — DONE
- GetNetworkRate early-return returning wrong counter (sendRate vs receiveRate)
- DiskCount.load() called on non-atomic plain int
- swprintf_s missing buffer size parameter
- Bar spacing now uses getTotalHeight() (bar + label text) instead of raw bar height
- Disabled disks excluded from Storage tile rendering
- PDH discovery switched from PdhEnumObjectItemsW to PdhExpandWildCardPathW (more reliable)
- CPU model text centering fixed (uses tile center, not content center)

## Architecture Notes

### Data Flow
```
updateStats thread  ──stores──>  StatsData (atomics)  ──loads──>  renderLoop thread
        │                                                              │
   StatsV1 methods                                              Tile rendering
   (PDH queries)                                                (GaugeV1, BarV1)
```

### PDH Timing
All rate-based counters use a non-blocking two-phase pattern:
1. First call: PdhCollectQueryData, record timestamp, return cached value
2. Subsequent calls: check elapsed time via QueryPerformanceCounter
3. If enough time passed: collect again, compute, cache, return

### Network Adapter Discovery (current)
1. `PdhExpandWildCardPathW(L"\\Network Interface(*)\\Bytes Total/sec")` to get all counter paths
2. Parse instance names from expanded paths between `(` and `)`
3. Dup-removal on parsed names
4. Keyword matching: Wi-Fi (wi-fi, wifi, wireless, wlan, 802.11)
5. Exclusion filters: bluetooth, virtual, loopback, teredo, isatap
6. Fallback: single adapter → assign to ethernet

### GPU Multi-Instance Architecture (current)
1. `PdhExpandWildCardPathW(L"\\GPU Engine(*)\\Utilization Percentage")` to get all counter paths
2. Parse instance names from expanded paths (skip `_Total`)
3. Create separate PDH query per GPU instance
4. Store in `vector<GPUInstance>` with per-instance cached utilization
5. `EnumDisplayDevicesW` loop maps display device names to `GPUInstance::displayName`
6. GPU tile renders one bar per instance labeled "GPU 1", "GPU 2"
7. displayName field exists on GPUInstance but not yet exposed via public API

### Tile Grid Layout (current: 4 cols × 6 rows)
```
Cell = (screenW / 4, (screenH - 30) / 6)
Default layout:
  Row 0: [CPU(2x2)]    [RAM(2x2)]
  Row 2: [GPU(2x2)]    [Network(2x2)]
  Row 4: [   Storage (4x2)   ]
```

### Tile Content Layout Algorithm
All content positioned within `[titleBar().bottom + 6, bounds.bottom - 8]`:
1. Model text (CPU/GPU): `min(bounds.w * 0.04, 16)` px font, auto-shrinks to fit width
2. Gauge: `min(cw * 0.48-0.58, contentH * xx%)` centered below model text
3. Bars: spaced using `bar.getTotalHeight()` as minimum, anchored from content bottom
4. Overflow guard: bars truncated if exceeding content bottom

## Known Limitations
- Single-window raylib (no multi-monitor or separate diagnostic window)
- Network: only one Wi-Fi and one Ethernet adapter monitored (not all adapters)
- No temperature, fan speed, or power monitoring
- No system tray minimize
- PDH GPU Engine counter may not exist on all systems (hybrid graphics, basic display adapters)
- Per-disk enabled state not persisted in config file
- Disabled disks still counted in GETDiskCount()
- Overlay fonts scale with window, not overlay panel dimensions
