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
├── main.cpp              # Entry point: global state, updateStats thread, renderLoop (orchestration),
│                         # F2/F3/F4 toggle, status bar, config save/load
├── LayoutConfig.h        # Centralized grid config: gridCols, gridRows, statusBarH, createDefaultTiles()
├── DesignSystem.h        # Theme→widget color mapping, typography helpers (tileTitleFont, modelFont)
├── TileRenderer.h        # Per-tile content: renderCPUTile/RAMTile/GPUTile/NetworkTile/StorageTile, StatsData
├── LandingPage.h         # Landing page: AppState enum, hero section, 3 theme cards, Start button
├── Overlays.h            # F2 diagnostics overlay, F3 settings overlay (scrollable frosted panels)
├── StatsV1.cpp / .h      # PDH stats collector: CPU freq/util, RAM, multi-GPU, multi-disk, network
├── BarV1.cpp / .h        # Horizontal progress bar: rounded ends, auto-scale, label truncation, drawInRect
├── GaugeV1.cpp / .h      # Circular arc gauge: 300-segment arc, glow, endpoint cap, ConfigArc/Quarter presets
├── ConfigV1.cpp / .h     # Config persistence: %APPDATA%\PerfMon\config.ini (INI format)
├── LoggerV1.cpp / .h     # CSV logger: F4 toggle, timestamped rows to working directory
├── TileV1.h              # Grid tile: drag, resize, snap-to-grid, collision avoidance, auto-span
├── ThemeV1.h             # 22-field color themes: Dark, Light, HighContrast static presets
└── tests/
    ├── test_main.cpp       # Test runner (63 tests, 7 groups)
    ├── test_harness.h      # Shared macros: TEST, CHECK, CHECK_EQ, CHECK_RANGE
    ├── test_cpu.cpp        # CPU tests (5)
    ├── test_ram.cpp        # RAM tests (4)
    ├── test_gpu.cpp        # GPU tests (4)
    ├── test_disk.cpp       # Disk tests (15)
    ├── test_network.cpp    # Network tests (5)
    └── test_graphics.cpp   # Widget tests: BarV1 (12), GaugeV1 (18)
```

## Phase History

### Phase 0: Scaffolding — DONE
- AGENTS.md, .gitignore, PLAN.md created

### Phase 1: Structural Consolidation — DONE
- Dead files removed, main.cpp created, CMakeLists.txt with FetchContent raylib 5.5
- Header/source separation for all components

### Phase 2: Robustness — DONE
- Eliminated all Sleep() calls (non-blocking QPC-timed PDH collection)
- Dynamic adapter/GPU discovery via PdhExpandWildCardPathW
- Thread safety: atomic cache, single-writer pattern

### Phase 3: Build & Distribution — DONE
- CMake targets: perfmon.exe + perfmon_tests.exe
- MSVC #pragma removed, unused includes cleaned

### Phase 4: GPU Monitoring — DONE
- PDH GPU Engine query, non-blocking two-phase pattern
- GPU/CPU model name via registry + EnumDisplayDevices

### Phase 5: Tile UI — DONE
- TileV1 widget: drag, resize, snap-to-grid, ghost preview, collision avoidance, auto-span

### Phase 6: Overlays — DONE
- F2 diagnostics (scrollable, Save button), F3 settings (device toggles), landing page with theme picker

### Phase 7: Testing — DONE
- 63 tests, 7 groups, 0 failures

### Phase 8: Feature Expansion — DONE
- Multi-GPU per-instance, per-disk/per-adapter enable/disable, config persistence, CSV logging

### Phase 9: UI Polish — DONE
- 4x6 grid, drop shadows, rounded corners, frosted overlays, status bar, text auto-shrinking

### Phase 10: Bug Fixes — DONE
- GetNetworkRate early-return, swprintf_s buffer size, bar spacing, PDH discovery robustness
- **GPU VRAM LUID matching:** Replaced index-based PDH VRAM counter assignment with LUID substring matching (issue: PDH `\GPU Adapter Memory(*)\Dedicated Usage` enumeration order differs from DXGI adapter order)
- **GPU model text:** Added missing model name rendering to `renderGPUTile` (matching `renderCPUTile` pattern)

### Phase 11: Production Consolidation — DONE
- Main.cpp split into LayoutConfig, DesignSystem, TileRenderer, LandingPage, Overlays
- Dead Makefile removed, CMakeLists.txt updated with all headers
- AGENTS.md rewritten with module dependency graph and updated file map
- README.md, PLAN.md, TODO.md cleaned

## Architecture Notes

### Data Flow
```
updateStats thread  ──stores──>  StatsData (atomics)  ──loads──>  renderLoop thread
        │                                                              │
   StatsV1 methods                                              TileRenderer.h
   (PDH queries)                                                (GaugeV1, BarV1)
```

### Module Dependency Graph
```
main.cpp
  ├── LayoutConfig.h          (grid constants, default tiles)
  ├── DesignSystem.h          (theme mapping, typography)
  │     ├── ThemeV1.h
  │     ├── BarV1.h
  │     └── GaugeV1.h
  ├── TileRenderer.h          (per-tile rendering)
  │     ├── StatsV1.h
  │     ├── BarV1.h
  │     └── GaugeV1.h
  ├── LandingPage.h           (landing page UI)
  ├── Overlays.h              (F2/F3 overlays)
  ├── TileV1.h                (tile widget)
  ├── ConfigV1.h              (config persistence)
  ├── LoggerV1.h              (CSV logging)
  └── StatsV1.h               (data collection)
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
5. Fallback: single adapter → ethernet

### GPU Multi-Instance Architecture
1. DXGI adapter enumeration for model + total VRAM + LUID
2. PDH `\GPU Adapter Memory(*)\Dedicated Usage` for VRAM usage, matched to GPU by LUID substring (case-insensitive)
3. WMI `Win32_PerfFormattedData_GPUPerformanceCounters_GPUEngine` for per-engine utilization, aggregated by LUID
4. WMI `Win32_VideoController` for clock speed
5. Sort GPUs by `vramTotalGB` descending (dedicated GPUs first) — LUID matching survives sort

### Tile Grid Layout (4 cols × 4 rows)

Grid is a fixed 4×4 matrix of cells split evenly based on window size. Each tile occupies a rectangular span of one or more cells.

```
Cell = (screenW / 4, (screenH - 30) / 4)
Default layout (5 tiles):
  Row 0: [CPU(2x2)]      [RAM(2x2)]
  Row 2: [GPU(2x2)]      [Network(2x2)]
  Row 3: [  Storage (4x1)  ]   <-- bottom row, full width
```

#### Tile Placement Algorithm

Each tile has a **footprint** (spanCols × spanRows cells). An occupancy map `bool grid[4][4]` tracks which cells are taken. The dragged tile is excluded from the map so it doesn't collide with itself.

**Placement logic when a tile is dropped at target (targetRow, targetCol):**

1. **Compute target footprint** — the cells `[targetRow .. targetRow+spanRows-1][targetCol .. targetCol+spanCols-1]`
2. **Check occupancy** — build the grid excluding the dragged tile. Are ALL cells in the target footprint free?
3. **IF all free:** Place tile directly. Done.
4. **IF cells occupied:** For each conflicting tile, attempt to **shift** it into the direction (up/down/left/right) that has the MOST contiguous free cells adjacent to it. Recurse: a shifted tile may conflict with another, so shift that too.
5. **IF any tile cannot be placed without overlapping or shrinking below its minimum footprint:** REJECT the move. The dragged tile returns to its original position. No partial/invalid state.

**Ghost preview:** While dragging, continuously compute:
- Where the dragged tile would land (accent outline)
- Where any displaced tiles would shift (dimmer translucent outline)
- If the move would be rejected, show the dragged tile at its original position with a "snap back" indicator

**Hard invariant:** Two tiles may NEVER occupy the same grid cell simultaneously, in preview or final state. Validated after every placement via exhaustive cell-by-cell check.

**Minimum tile sizes (cells):**
- CPU: minimum 1×2 (can't be narrower than 1 col or shorter than 2 rows)
- RAM: minimum 1×1
- GPU: minimum 1×2
- Network: minimum 1×2
- Storage: minimum 2×1

#### Test Scenarios

**Scenario 1 — Clean fit (no conflict):**
Grid is empty except CPU at (0,0,2×2) and RAM at (0,2,2×2). User drags Network to (2,0,2×2). Target cells (row 2-3, col 0-1) are vacant. Result: Network placed at (2,0,2×2). Verified: no cell overlaps CPU(0,0) or RAM(0,2).

**Scenario 2 — Expand-and-displace (shift to make room):**
Grid has CPU(0,0,2×2), RAM(0,2,2×2), Network(2,0,2×2). User drags GPU to (0,0) wanting 2×2. Target overlaps CPU. Above CPU is edge (row 0). Below CPU: rows 2-3 are occupied by Network. Left: edge. Right: RAM at (0,2). No contiguous 2×2 free space. So attempt to shift CPU right into RAM's space. RAM(0,2) can shift down if space exists. If rows 2-3 col 2-3 are free, RAM shifts to (2,2,2×2). Then CPU shifts to (0,2,2×2). GPU placed at (0,0,2×2). Verified: no overlaps.

**Scenario 3 — Rejected move (CPU minimum 1×2 violation):**
CPU minimum size is 1×2. Grid has tiles filling all cells. User drags CPU toward (0,0) but required 2×2 cells aren't free. The only way to make room would require shrinking CPU below its 1×2 minimum (e.g., to 1×1). Move is REJECTED. CPU snaps back to original position. Verified: CPU stays at original footprint, no cells overlapping.

## Known Limitations
- Single-window raylib (no multi-monitor or separate diagnostic window)
- Network: only one Wi-Fi and one Ethernet adapter monitored
- No temperature, fan speed, or power monitoring
- No system tray minimize
- Per-disk enabled state not persisted in config file
- Disabled disks still counted in GETDiskCount()
- GPU utilization requires WMI availability
