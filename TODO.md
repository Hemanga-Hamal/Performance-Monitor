# TODO â€” Performance Monitor

## Status

Build: `cmake -B build && cmake --build build --config Release`
Tests: `build\Release\perfmon_tests.exe` â€” 63 passed, 0 failed

---

## Completed (Phases 0â€“12)

All major features complete: CPU/RAM/GPU/Disk/Network monitoring, tile dashboard with drag-to-arrange, 3 themes with config persistence, F2 diagnostics, F3 settings, F4 CSV logging, 63-test suite, production-ready module split with centralized design system.

### [x] GPU VRAM counter cross-wired between GPUs
**Files:** `Stats.cpp` (PDH VRAM setup)
**Fix:** PDH VRAM counter attachment now happens BEFORE the GPU sort by vramTotalGB. Previously, PDH `\GPU Adapter Memory(*)\Dedicated Usage` counters were attached after the sort, causing physical-adapter-index mismatches (Intel iGPU got NVIDIA's VRAM counter and vice versa).

### [x] GPU model text missing from tile
**Files:** `Rendering.h` (renderGPUTile)
**Fix:** Model text removed from GPU tile (simplified to gauge + 2 bars). GPU tile now shows utilization gauge + VRAM bar + Clock bar.

### [x] GPU utilization always 0 (PDH `_Total` counter missing)
**Files:** `Stats.cpp` (GETGPUUtilization, QueryGpuUtilWmi, InitWbem)
**Fix:** Replaced PDH `\GPU Engine` per-process counter approach with WMI `Win32_PerfFormattedData_GPUPerformanceCounters_GPUEngine` â€” aggregates all per-engine utilization by LUID. PDH GPU Engine counters (`_Total` aggregate) don't exist on many Windows 10/11 systems, and per-process counters return 0 at idle. WMI approach sums all engine instances per GPU for a meaningful aggregate.

### [x] GPU discovery via DXGI-first (always works)
**Files:** `Stats.cpp` (constructor GPU init)
**Fix:** GPU instances now created from DXGI `EnumAdapters` (always available), with PDH VRAM and WMI utilization as optional layers. Ensures GPU tile always shows model + total VRAM even if PDH/WMI fail.

### [x] RAM tile missing used/total bar
**Files:** `Rendering.h` (renderRAMTile), `WindowManager.h` (collectBarData, renderTileContent)
**Fix:** Added bar showing `used / total GB` beneath the gauge, splitting content 50/50. Uses `bars[15]`.

### [x] F2 diagnostics GPU section formatting
**Files:** `Overlays.h` (drawDiagnosticsOverlay)
**Fix:** VRAM and Clock lines now have proper "VRAM:" / "Clock:" labels in white (textPrimary), matching other section formatting.

### [x] GPU gauge value not set consistently
**Files:** `WindowManager.h` (collectBarData), `Rendering.h` (renderGPUTile)
**Fix:** `gaugeGPU.setValue()` moved to `collectBarData` (alongside CPU/RAM gauges). Previously only set in `renderGPUTile`, risking missed updates.

---

## Known Issues

### [ ] CPU frequency reports 0 on some systems
**Files:** `Stats.cpp` (GETCPUFrequency)
**Symptoms:** CPU frequency bar shows 0.0. CPU utilization still works.
**Fix:** Fallback to registry base frequency or `\Processor(_Total)\% Processor Performance`.

### [âš  fixed] GPU utilization + VRAM not working on many systems
**Fix:** DXGI-first GPU discovery + WMI `Win32_PerfFormattedData_GPUPerformanceCounters_GPUEngine` aggregation replaces PDH `\GPU Engine(*)` (no `_Total` aggregate, per-process counters near zero). PDH VRAM counter attachment order fixed (before sort).

### [ ] CPU model text truncated for very long names
**Files:** `Rendering.h` (renderCPUTile)
**Symptoms:** Font auto-shrinks to a minimum; at small tile sizes text unreadable.
**Fix:** Multi-line text or marquee scrolling on small tiles.

### [ ] Network adapter names contain PDH instance junk
**Files:** `Stats.cpp` (FindNetworkAdapters)
**Fix:** Clean names or use IP Helper API (GetAdaptersAddresses) for friendly names.

### [ ] Per-disk enabled state not persisted in config
**Files:** `Config.h/.cpp`, `main.cpp`
**Fix:** Save/load disk/adapter enabled arrays.

### [ ] Disabled disks still counted in `GETDiskCount()`
**Files:** `Stats.h`
**Fix:** Add `GETEnabledDiskCount()`.

### [ ] F3 per-adapter toggles don't affect data collection
**Files:** `main.cpp` (updateStats), `Stats.h/.cpp`
**Fix:** Add per-adapter enabled checks in GETWiFi/GETEthernet methods.

### [ ] Resize grip hitbox (16x16) small on high-DPI
**Files:** `Tile.h` (handleDrag)
**Fix:** Scale grip hitbox by screen DPI.

---

## Future Work

### [ ] GPU display name in tile bar labels
**Priority:** Low | **Files:** `Stats.h/.cpp`, `TileRenderer.h`
GPUInstance::displayName exists but not yet exposed. Add `GetGPUDisplayName(int)` and use in GPU tile.

### [ ] Temperature monitoring (CPU/GPU)
**Priority:** Medium | **Files:** `Stats.h/.cpp`
Via registry, MSR, nvapi/ADL, or WMI. Not all systems expose thermal data.

### [ ] System tray minimize
**Priority:** Medium | **Files:** `main.cpp`
Use `Shell_NotifyIconW`, hook WM_SIZE via raylib's GLFW window.

### [ ] CPU utilization history sparkline
**Priority:** Medium | **Files:** `TileRenderer.h` (renderCPUTile)
Store last N values in ring buffer, draw polyline below gauge.

### [ ] Configurable grid layout
**Priority:** Medium | **Files:** `LayoutConfig.h`, `Config.h`
Move grid config to config file, allow dynamic tile add/remove.

### [ ] Dark/Light mode auto-detection
**Priority:** Low | **Files:** `main.cpp`
Read `HKCU\...\AppsUseLightTheme` registry value at startup.

### [ ] Bar color gradient fill
**Priority:** Low | **Files:** `Bar.cpp`
Use `DrawRectangleGradientH` with endColor in Theme.

### [ ] Window resize snapping / preset sizes
**Priority:** Low | **Files:** `main.cpp`
Snap window size to cell-multiples, or preset sizes (Compact/Normal/Large).

### [ ] Hover tooltips for bar labels
**Priority:** Low | **Files:** `TileRenderer.h`
On hover, draw tooltip with full label text.
