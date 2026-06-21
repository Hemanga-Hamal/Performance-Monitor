# Project Plan: Performance Monitor

## Status: Phase 17 — Structural Rebuild + GPU Fix

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
├── main.cpp              # Entry point, 2 threads, tile dashboard rendering, status bar,
│                         # F4 CSV logging, preset bar (F5), tooltips, event handling
├── LandingPage.cpp/.h    # Landing page: theme picker (3 cards), "Start Monitoring" button
│                         # Defines enum AppState { LANDING, DASHBOARD }
├── DiagnosticsOverlay.cpp/.h  # F2 diagnostics: frosted overlay, scroll, sections, Save
├── SettingsOverlay.cpp/.h     # F3 settings: tile/disk/adapter toggle checkboxes
├── StatsData.h           # Shared StatsData struct (atomics) for cross-thread sharing
├── StatsCollector.cpp / .h      # PDH + Win32 stats collector
├── BarWidget.cpp / .h        # Horizontal progress bar widget (with Theme::fromAppTheme factory)
├── GaugeWidget.cpp / .h      # Circular arc gauge widget (with Theme::fromAppTheme factory)
├── ConfigManager.cpp / .h     # Config persistence to %APPDATA%\PerfMon\config.ini (INI format)
├── CsvLogger.cpp / .h     # CSV logger: F4 toggle, writes timestamped metric rows
├── TileItem.h              # Grid tile widget with drag-to-swap, GridLayout manager
│                         # GridConfig/GridPresets: 5 grid sizes, F5 toggle
├── AppTheme.h             # Canonical 22-field color theme (Dark, Light, HighContrast)
├── TitleBar.h / .cpp     # Custom 32px title bar, drag + close
├── TitleBarWin32.cpp     # Win32 subclass: WM_NCHITTEST→HTCAPTION
└── tests/
    ├── test_main.cpp       # Test runner (calls all 7 groups)
    ├── test_harness.h      # Shared macros: TEST, CHECK, CHECK_EQ, CHECK_RANGE
    ├── test_cpu.cpp        # CPUTests: frequency, utilization, model name (5 tests)
    ├── test_ram.cpp        # RAMTests: total, used, utilization (4 tests)
    ├── test_gpu.cpp        # GPUTests: available, utilization, model, name (4 tests)
    ├── test_disk.cpp       # DiskTests: count, per-disk name/total/used/util (15 tests)
    ├── test_network.cpp    # NetworkTests: WiFi/Ethernet send/receive, adapter list (5 tests)
    └── test_graphics.cpp   # GraphicsTests: BarWidget/GaugeWidget: theme, dims, config, sizing (30 tests)
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
- Dynamic network adapter discovery via PdhExpandWildCardPathW
- Thread safety: atomic cache values, single-writer pattern

### Phase 3: Build & Distribution — DONE
- CMakeLists.txt targets: perfmon.exe + perfmon_tests.exe
- MSVC #pragma removed, linking in CMake target_link_libraries
- GaugeWidget missing setters implemented
- README.md updated

### Phase 4: GPU Monitoring — DONE
- PDH GPU Engine query: `\GPU Engine(*)\Utilization Percentage`
- Non-blocking two-phase pattern
- GPU model name via EnumDisplayDevices
- CPU model name via registry

### Phase 5: Tile UI — DONE
- TileItem widget: computeBounds, handleDrag, snapToGrid, drawFrame, snapPreview
- Drag-to-swap, ghost preview, collision avoidance
- Auto-span recomputed from bounds/cell ratio

### Phase 6: Overlays — DONE
- F2: Scrollable diagnostics with scissor clipping, section headers, Save button
- F3: Device visibility toggles with per-disk/per-adapter sections
- Landing page: theme selector (3 cards with color swatches), Start button

### Phase 7: Testing — DONE
- 63 tests, 7 groups, 0 failures

### Phase 8: Feature Expansion — DONE
- Multi-GPU per-instance monitoring
- Per-disk and per-adapter enable/disable
- BarWidget::drawInRect and GaugeWidget::drawInRect
- Config file persistence (INI in %APPDATA%\PerfMon\config.ini)
- CSV logging with F4 toggle
- Export diagnostics to text file

### Phase 9: UI Polish — DONE
- Migrated from 2x4 to 4x6 grid for finer snapping
- Drop shadows on tiles and overlays
- BarWidget rounded ends, GaugeWidget glow + endpoint cap
- Frosted glass overlay panels
- Rounded checkbox toggles in F3
- Status bar with background + separator
- Text auto-shrinking, font sizes from tile dimensions
- Network/GPU labels simplified

### Phase 10: Bug Fixes — DONE
- GetNetworkRate early-return wrong counter
- DiskCount.load() on non-atomic int
- swprintf_s missing buffer size
- Bar spacing uses getTotalHeight()
- PDH discovery switched to PdhExpandWildCardPathW

### Phase 11: Feature & Polish — DONE
- CPU frequency registry fallback
- CPU model multi-line text wrapping
- GPU per-instance display names
- Per-disk/per-adapter enabled state persisted
- BarWidget gradient fill, hover tooltips
- Dark/Light auto-detection on first run
- Network adapter name cleaning
- Tile drag/resize fully functional

### Phase 12: Custom Title Bar & UI — DONE
- FLAG_WINDOW_UNDECORATED with custom 32px themed title bar
- Title bar on both landing page and dashboard
- boundsYOffset for grid offset

### Phase 13: Code Quality Refactor — DONE
- main.cpp split into modular components:
  - LandingPage.cpp/.h — landing page rendering
  - DiagnosticsOverlay.cpp/.h — F2 diagnostics panel
  - SettingsOverlay.cpp/.h — F3 settings panel
  - StatsData.h — shared atomic struct for cross-thread data
- GaugeWidget.h include guard fixed: `GaugeWidget_H` → `GaugeWidget_H`
- Design system unifications: BarWidget::Theme::fromAppTheme() and GaugeWidget::Theme::fromAppTheme() factory methods
- main.cpp: 1189 → 740 lines

### Phase 14: VS Code-Style Docking Layout — DISCARDED
- LayoutTree BSP docking had fundamental bugs (fall-through, freeform drag incompatibility)
- Entire dashboard rebuilt in Phase 15

### Phase 15: Grid-Based Tile System — DONE
- Clean-slate rebuild: GridLayout manager with drag-to-swap
- Tiles snap to grid cells (4x6), index-based dispatch
- Auto-scaling contentScale per tile

### Phase 17: Structural Rebuild + GPU Fix + Tile Algorithm — IN PROGRESS
- New tile placement algorithm (footprint-based, expand-displace, hard reject, full ghost preview)
- 4×4 fixed grid replacing old 4×6 system
- Code reorganized into module-per-system: CPU, GPU, RAM, Disk, Network, Layout, Rendering, Window
- Tile as primary rendering context — all content governed by parent tile
- GPU data fixed definitively: PDH + DXGI for model/util/VRAM/clock
- Design simplified to clean minimalism

## Architecture Notes

### Data Flow
```
updateStats thread  ──stores──>  StatsData (atomics)  ──loads──>  renderLoop thread
        │                                                              │
   StatsCollector methods                                              Tile rendering
   (PDH queries)                                                (GaugeWidget, BarWidget)
```

### PDH Timing
All rate-based counters use a non-blocking two-phase pattern:
1. First call: PdhCollectQueryData, record timestamp, return cached value
2. Subsequent calls: check elapsed time via QueryPerformanceCounter
3. If enough time passed: collect again, compute, cache, return

### Network Adapter Discovery
1. `PdhExpandWildCardPathW(L"\\Network Interface(*)\\Bytes Total/sec")` to get all counter paths
2. Parse instance names from expanded paths between `(` and `)`
3. Keyword matching: Wi-Fi (wi-fi, wifi, wireless, wlan, 802.11)
4. Exclusion filters: bluetooth, virtual, loopback, teredo, isatap
5. Fallback: single adapter → assign to ethernet

### GPU Multi-Instance Architecture
1. `PdhExpandWildCardPathW(L"\\GPU Engine(*)\\Utilization Percentage")` to get all counter paths
2. Parse instance names from expanded paths (skip `_Total`)
3. Create separate PDH query per GPU instance
4. Store in `vector<GPUInstance>` with per-instance cached utilization
5. Render one bar per instance labeled "GPU 1", "GPU 2"

### Tile Placement Algorithm — Precise Spec

Grid is a fixed 4×4 matrix. Cells split evenly based on current window size. Think of it as a 4×4 matrix of cell coordinates (row, col), each 0–3.

A tile occupies a rectangular span of one or more cells (e.g. a 2×2 tile occupies 4 cells). Each tile has per-category minimum/maximum size constraints.

**PLACEMENT LOGIC — when a tile is dropped at a target cell position:**

1. Determine dragged tile's footprint (spanCols × spanRows) from its current size setting.
2. Check whether ALL cells required for that footprint, starting at the drop target position, are currently unoccupied (ignoring the dragged tile itself).
3. **IF all required cells are free:** place the tile there directly. Done.
4. **IF one or more required cells are occupied by another tile:** attempt to make room by expanding/shifting available free space toward whichever direction (up/down/left/right) currently has the MOST contiguous free cells, displacing the conflicting tile(s) into that space, but ONLY if the conflicting tile(s) can fit in the resulting space without violating their own minimum size.
5. **IF no valid arrangement exists** that accommodates the move without forcing any tile below its minimum size or causing overlap: **REJECT** the move entirely. The dragged tile returns to its original position. No partial or invalid swap occurs.

**GHOST PREVIEW:** while dragging (before drop), continuously compute and display a live preview of the full prospective outcome — not just a highlight of the target cell. Show where the dragged tile would land AND how any displaced tiles would shift/resize as a result, updating in real time as the user drags. The user sees the complete resulting layout before committing to the drop.

**HARD INVARIANT:** two tiles may never occupy any of the same grid cell(s) simultaneously, at any point, in the live preview's underlying computed state or in the final committed layout. Validate this after every placement.

**Test Scenario 1 — Clean Fit:**
Grid has CPU(2×2) at (0,0), RAM(2×2) at (2,0), GPU(2×1) at (0,2), Network(2×1) at (2,2), Storage(4×1) at (0,3). User drags CPU to (2,2). Network occupies (2,2). GPU is at (0,2). The direction with most free space is UP/LEFT — Network shifts to (0,2) and GPU shifts to (2,2). Clean swap of the two right-column tiles. Result: CPU at (2,2), GPU at (0,2), Network at (2,2)... wait, that's a conflict. Let me redo. Actually simpler: drag CPU(2×2) to drop at (2,0) where RAM(2×2) sits. RAM can move to (0,0) which is now free (CPU left it). Result: RAM at (0,0), CPU at (2,0). Clean swap. No min-size violations.

**Test Scenario 2 — Expand-and-Displace:**
Grid has CPU(2×2) at (0,0), RAM(2×2) at (2,0), GPU(2×1) at (0,2), Network(2×1) at (2,2), Storage(4×1) at (0,3). User drags CPU(2×2) to drop at (0,1). Cells (0,1)-(1,2) are: GPU occupies (0,2)-(1,2) and CPU's own old position is (0,0)-(1,1). The conflicting tile is GPU. Check all 4 directions for contiguous free space: DOWN has Storage but row 4+ is off-grid. RIGHT has RAM then Network. UP has nothing (row -1). LEFT has nothing (col -1). The most free cells are DOWN — but GPU can't move there without pushing Storage. Actually simpler scenario: drag GPU(2×1) to (0,0). CPU is there. CPU needs 2×2 minimum. The only free cells are (2,0)-(3,1) (where CPU would need to move). CPU(2×2) fits at (2,0)-(3,1). Result: GPU at (0,0), CPU at (2,0). RAM is still at (2,0)... conflict! RAM occupies (2,0)-(3,1). So can't place CPU at (2,0). The move is REJECTED because CPU has minimum 2×2 and the only space large enough is occupied by RAM, and RAM(min 2×2) can't fit at (0,0)-(1,1) which is only 2×1. Reject.

**Test Scenario 3 — Rejected Case (CPU Minimum Violation):**
Grid: CPU(2×2) at (0,0), RAM(2×2) at (2,0), GPU(2×1) at (0,2), Network(2×1) at (2,2), Storage(4×1) at (0,3). User drags GPU(2×1) to drop at (0,0). CPU occupies (0,0)-(1,1). CPU minimum is 2×2. Can CPU move anywhere with 2×2 free? Check all 4×4 cells ignoring GPU: occupied cells are RAM(2,0)-(3,1), Network(2,2)-(3,2), Storage(0,3)-(3,3). Free cells: none large enough for 2×2. REJECT. GPU snaps back to (0,2). Nothing changes.

### Tile Grid Layout (default: 4×4)
```
Cell = (screenW / 4, (screenH - statusBar) / 4)
Default layout:
  Row 0: [CPU(2×2)]    [RAM(2×2)]
  Row 2: [GPU(2×1)]    [Network(2×1)]
  Row 3: [   Storage (4×1)   ]
```

### Design System
- **AppTheme**: Canonical 22-field color theme. Dark, Light, HighContrast static factories.
- **BarWidget::Theme::fromAppTheme(activeTheme)**: Maps barBackground, barForeground, barForegroundDim, textPrimary
- **GaugeWidget::Theme::fromAppTheme(activeTheme)**: Maps tileBg, gaugeArcBg, gaugeArcActive, gaugeText

## Known Limitations
- Single-window (no multi-monitor or separate diagnostic window)
- Network: only one Wi-Fi and one Ethernet adapter monitored
- No temperature, fan speed, or power monitoring
- No system tray minimize
- PDH GPU Engine counter may not exist on all systems
- Window resize snapping to grid cell multiples not implemented
