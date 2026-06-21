# Project Plan: Performance Monitor

## Status: Phase 13

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
â”œâ”€â”€ main.cpp              # Entry point (~10 lines): just calls WindowManager::run()
â”œâ”€â”€ WindowManager.h       # Application orchestrator: global state, config load/save, updateStats thread,
â”‚                         # renderLoop (state machine, F2/F3/F4 dispatch, status bar, config save on exit)
â”œâ”€â”€ Rendering.h           # Unified rendering: DesignSystem namespace, StatsData, formatValue,
â”‚                         # renderCPUTile, renderRAMTile, renderGPUTile, renderNetworkTile, renderStorageTile
â”œâ”€â”€ LandingPage.h         # Landing page: AppState enum, hero section, 3 theme cards, Start button
â”œâ”€â”€ Overlays.h            # F2 diagnostics overlay, F3 settings overlay (scrollable frosted panels)
â”œâ”€â”€ Tile.h              # Tile grid system: TileConfig, LayoutConfig (4x4 grid constants),
â”‚                         # Tile class (cell-based occupancy, drag/resize, displacement cascade, etc.)
â”œâ”€â”€ Theme.h             # 22-field color themes: Dark, Light, HighContrast static presets
â”œâ”€â”€ DataTypes.h           # Shared types: DiskInfo, AdapterInfo, GPUInstance
â”œâ”€â”€ Stats.cpp / .h      # PDH + DXGI + WMI stats collector
â”‚                         # CPU: frequency (PDH), utilization (GetSystemTimes), model (registry)
â”‚                         # RAM: total/used/util (GlobalMemoryStatusEx)
â”‚                         # GPU: utilization (WMI per-LUID aggregation), VRAM (PDH dedicated by physical index),
â”‚                         #   total VRAM (DXGI), model (DXGI desc), clock (WMI Win32_VideoController)
â”‚                         # Disk: all DRIVE_FIXED, per-disk enabled/disabled
â”‚                         # Network: WiFi/Ethernet via PdhExpandWildCardPathW, keyword filter
â”œâ”€â”€ Bar.cpp / .h        # Horizontal progress bar widget
â”œâ”€â”€ Gauge.cpp / .h      # Circular arc gauge widget (300-segment triangle arc)
â”œâ”€â”€ Config.cpp / .h     # Config persistence: %APPDATA%\PerfMon\config.ini (INI format)
â”œâ”€â”€ Logger.cpp / .h     # CSV logger: F4 toggle, timestamped rows to working directory
â””â”€â”€ tests/
    â”œâ”€â”€ test_main.cpp       # Test runner (63 tests, 7 groups)
    â”œâ”€â”€ test_harness.h      # Shared macros: TEST, CHECK, CHECK_EQ, CHECK_RANGE
    â”œâ”€â”€ test_cpu.cpp        # CPU tests (5)
    â”œâ”€â”€ test_ram.cpp        # RAM tests (4)
    â”œâ”€â”€ test_gpu.cpp        # GPU tests (4)
    â”œâ”€â”€ test_disk.cpp       # Disk tests (15)
    â”œâ”€â”€ test_network.cpp    # Network tests (5)
    â””â”€â”€ test_graphics.cpp   # Widget tests: Bar (12), Gauge (18)
```

## Phase History

### Phase 0: Scaffolding â€” DONE
- AGENTS.md, .gitignore, PLAN.md created

### Phase 1: Structural Consolidation â€” DONE
- Dead files removed, main.cpp created, CMakeLists.txt with FetchContent raylib 5.5
- Header/source separation for all components

### Phase 2: Robustness â€” DONE
- Eliminated all Sleep() calls (non-blocking QPC-timed PDH collection)
- Dynamic adapter/GPU discovery via PdhExpandWildCardPathW
- Thread safety: atomic cache, single-writer pattern

### Phase 3: Build & Distribution â€” DONE
- CMake targets: perfmon.exe + perfmon_tests.exe
- MSVC #pragma removed, unused includes cleaned

### Phase 4: GPU Monitoring â€” DONE
- PDH GPU Engine query, non-blocking two-phase pattern
- GPU/CPU model name via registry + EnumDisplayDevices

### Phase 5: Tile UI â€” DONE
- Tile widget: drag, resize, snap-to-grid, ghost preview, collision avoidance, auto-span

### Phase 6: Overlays â€” DONE
- F2 diagnostics (scrollable, Save button), F3 settings (device toggles), landing page with theme picker

### Phase 7: Testing â€” DONE
- 63 tests, 7 groups, 0 failures

### Phase 8: Feature Expansion â€” DONE
- Multi-GPU per-instance, per-disk/per-adapter enable/disable, config persistence, CSV logging

### Phase 9: UI Polish â€” DONE
- 4x6 grid, drop shadows, rounded corners, frosted overlays, status bar, text auto-shrinking

### Phase 10: Bug Fixes â€” DONE
- GetNetworkRate early-return, swprintf_s buffer size, bar spacing, PDH discovery robustness
- **GPU VRAM LUID matching:** Replaced index-based PDH VRAM counter assignment with LUID substring matching (issue: PDH enumeration order differs from DXGI adapter order)
- **GPU model text:** Added missing model name rendering to `renderGPUTile` (matching `renderCPUTile` pattern)

### Phase 11: Production Consolidation â€” DONE
- Extracted LayoutConfig into Tile.h, DesignSystem + TileRenderer into Rendering.h
- Extracted DataTypes.h from Stats.h, WindowManager.h from main.cpp
- main.cpp slimmed to ~10 lines
- Removed dead Makefile, updated CMakeLists.txt
- AGENTS.md rewritten with module dependency graph, PLAN.md and TODO.md cleaned

### Phase 12: GPU Stats Reliability â€” DONE
- Root cause: `\GPU Engine(_Total)\Utilization Percentage` does not exist on many Windows 10/11 systems; per-process GPU engine counters return 0 at idle
- Replaced PDH per-process engine approach with WMI `Win32_PerfFormattedData_GPUPerformanceCounters_GPUEngine` â€” aggregates all per-LUID engine utilization
- GPU discovery restructured DXGI-first (always works), with PDH VRAM + WMI utilization as optional layers
- Fixed PDH VRAM counter attachment order (must be before GPU sort by vramTotalGB to avoid cross-wiring)
- Added persistent WMI connection (`IWbemLocator` + `IWbemServices`) for periodic utilization queries
- Added case-insensitive LUID matching between PDH/WMI counter paths
- Added GPU sort by vramTotalGB descending (dedicated GPU always at index 0)
- GPU tile: simplified to gauge (utilization %) + 2 bars (VRAM, Clock)
- RAM tile: added used/total GB bar below gauge
- F2 diagnostics: fixed GPU section labels (VRAM/Clock now have proper labels)
- `gaugeGPU.setValue()` moved to `collectBarData` for consistent gauge updates

### Phase 13: Module Rename — DONE
- Stripped V1 suffix from all module class names, file names, and references
- Renames: StatsV1 → Stats, BarV1 → Bar, GaugeV1 → Gauge, TileV1 → Tile, ThemeV1 → Theme, ConfigV1 → Config, LoggerV1 → Logger
- Updated all documentation references in AGENTS.md, TODO.md, and PLAN.md

## Architecture Notes

### Data Flow
```
updateStats thread  â”€â”€storesâ”€â”€>  StatsData (atomics)  â”€â”€loadsâ”€â”€>  renderLoop thread
        â”‚                                                              â”‚
   Stats methods                                              TileRenderer.h
    (PDH queries)                                                (Gauge, Bar)
```

### Module Dependency Graph
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
        â”œâ”€â”€ Overlays.h      (F2/F3 panels)
        â”œâ”€â”€ Config.h      (config persistence)
        â””â”€â”€ Logger.h      (CSV logging)

DataTypes.h (standalone â€” Win32/PDH types)
Stats.h â†’ DataTypes.h
Tests/ â†’ all src/ modules
```

### PDH Timing
All rate-based counters use a non-blocking two-phase pattern:
1. First call: PdhCollectQueryData, record timestamp, return cached value
2. Subsequent calls: check elapsed time via QueryPerformanceCounter
3. Proceed only if enough time passed: collect again, compute, cache, return

### Network Adapter Discovery
1. `PdhExpandWildCardPathW` to get all `\Network Interface(*)\Bytes Total/sec` counter paths
2. Parse instance names, deduplicate
3. Wi-Fi detection: keyword match (wi-fi, wifi, wireless, wlan, 802.11)
4. Exclude: bluetooth, virtual, loopback, teredo, isatap
5. Fallback: single adapter â†’ ethernet

### GPU Multi-Instance Architecture
1. DXGI adapter enumeration for model + total VRAM + LUID (always available)
2. PDH `\GPU Adapter Memory(*)\Dedicated Usage` for VRAM usage, matched by physical adapter index (attached BEFORE sort)
3. WMI `Win32_PerfFormattedData_GPUPerformanceCounters_GPUEngine` for per-engine utilization, aggregated by LUID (replaces unreliable PDH `\GPU Engine` which lacks `_Total` aggregate)
4. WMI `Win32_VideoController` for clock speed
5. Persistent WMI connection for periodic GPU utilization queries
6. Sort GPUs by `vramTotalGB` descending (dedicated GPUs first) â€” PDH VRAM counters attached before sort to avoid cross-wiring

### Tile Grid Layout (4 cols Ã— 4 rows)

Grid is a fixed 4Ã—4 matrix of cells split evenly based on window size. Each tile occupies a rectangular span of one or more cells.

```
Cell = (screenW / 4, (screenH - 30) / 4)
Default layout (5 tiles):
  Row 0: [CPU(2x2)]      [GPU(2x2)]
  Row 2: [RAM(2x1)]      [Network(2x1)]
  Row 3: [  Storage (4x1)  ]   <-- bottom row, full width
```

#### Tile Placement Algorithm

Each tile has a **footprint** (spanCols Ã— spanRows cells). An occupancy map `bool grid[4][4]` tracks which cells are taken. The dragged tile is excluded from the map so it doesn't collide with itself.

**Placement logic when a tile is dropped at target (targetRow, targetCol):**

1. **Compute target footprint** â€” the cells `[targetRow .. targetRow+spanRows-1][targetCol .. targetCol+spanCols-1]`
2. **Check occupancy** â€” build the grid excluding the dragged tile. Are ALL cells in the target footprint free?
3. **IF all free:** Place tile directly. Done.
4. **IF cells occupied:** For each conflicting tile, attempt to **shift** it into the direction (up/down/left/right) that has the MOST contiguous free cells adjacent to it. Recurse: a shifted tile may conflict with another, so shift that too.
5. **IF any tile cannot be placed without overlapping or shrinking below its minimum footprint:** REJECT the move. The dragged tile returns to its original position. No partial/invalid state.

**Ghost preview:** While dragging, continuously compute:
- Where the dragged tile would land (accent outline)
- Where any displaced tiles would shift (dimmer translucent outline)
- If the move would be rejected, show the dragged tile at its original position with a "snap back" indicator

**Hard invariant:** Two tiles may NEVER occupy the same grid cell simultaneously, in preview or final state. Validated after every placement via exhaustive cell-by-cell check.

**Minimum tile sizes (cells):**
- CPU: minimum 1Ã—2 (can't be narrower than 1 col or shorter than 2 rows)
- RAM: minimum 1Ã—1
- GPU: minimum 1Ã—2
- Network: minimum 1Ã—2
- Storage: minimum 2Ã—1

#### Test Scenarios

**Scenario 1 â€” Clean fit (no conflict):**
Grid is empty except CPU at (0,0,2Ã—2) and RAM at (0,2,2Ã—2). User drags Network to (2,0,2Ã—2). Target cells (row 2-3, col 0-1) are vacant. Result: Network placed at (2,0,2Ã—2). Verified: no cell overlaps CPU(0,0) or RAM(0,2).

**Scenario 2 â€” Expand-and-displace (shift to make room):**
Grid has CPU(0,0,2Ã—2), RAM(0,2,2Ã—2), Network(2,0,2Ã—2). User drags GPU to (0,0) wanting 2Ã—2. Target overlaps CPU. Above CPU is edge (row 0). Below CPU: rows 2-3 are occupied by Network. Left: edge. Right: RAM at (0,2). No contiguous 2Ã—2 free space. So attempt to shift CPU right into RAM's space. RAM(0,2) can shift down if space exists. If rows 2-3 col 2-3 are free, RAM shifts to (2,2,2Ã—2). Then CPU shifts to (0,2,2Ã—2). GPU placed at (0,0,2Ã—2). Verified: no overlaps.

**Scenario 3 â€” Rejected move (CPU minimum 1Ã—2 violation):**
CPU minimum size is 1Ã—2. Grid has tiles filling all cells. User drags CPU toward (0,0) but required 2Ã—2 cells aren't free. The only way to make room would require shrinking CPU below its 1Ã—2 minimum (e.g., to 1Ã—1). Move is REJECTED. CPU snaps back to original position. Verified: CPU stays at original footprint, no cells overlapping.

## Known Limitations
- Single-window raylib (no multi-monitor or separate diagnostic window)
- Network: only one Wi-Fi and one Ethernet adapter monitored
- No temperature, fan speed, or power monitoring
- No system tray minimize
- Per-disk enabled state not persisted in config file
- Disabled disks still counted in GETDiskCount()
- GPU utilization requires WMI availability
