# TODO — Performance Monitor

## Status

Build: `cmake -B build && cmake --build build --config Release`
Tests: `build\Release\perfmon_tests.exe` — 63 passed, 0 failed

---

## Completed (Phases 0–11)

All major features complete: CPU/RAM/GPU/Disk/Network monitoring, tile dashboard with drag-to-arrange, 3 themes with config persistence, F2 diagnostics, F3 settings, F4 CSV logging, 63-test suite, production-ready module split with centralized design system.

### [x] GPU VRAM counter cross-wired between GPUs
**Files:** `StatsV1.cpp` (PDH VRAM setup)
**Fix:** Replaced index-based matching with LUID-based matching for `\GPU Adapter Memory(*)\Dedicated Usage` PDH counters (same approach as `QueryGpuUtilWmi`). PDH enumeration order can differ from DXGI adapter order; LUID substring matching ensures each GPU gets the correct VRAM counter regardless of enumeration order or post-sort reordering.

### [x] GPU model text missing from tile
**Files:** `Rendering.h` (renderGPUTile)
**Fix:** Added model name text rendering above the gauge in `renderGPUTile`, matching the `renderCPUTile` model text pattern (auto-shrinking font, `theme.textSecondary` color).

---

## Known Issues

### [ ] CPU frequency reports 0 on some systems
**Files:** `StatsV1.cpp` (GETCPUFrequency)
**Symptoms:** CPU frequency bar shows 0.0. CPU utilization still works.
**Fix:** Fallback to registry base frequency or `\Processor(_Total)\% Processor Performance`.

### [ ] CPU model text truncated for very long names
**Files:** `TileRenderer.h` (renderCPUTile)
**Symptoms:** Font auto-shrinks to a minimum; at small tile sizes text unreadable.
**Fix:** Multi-line text or marquee scrolling on small tiles.

### [ ] GPU PDH counter may not exist on all systems
**Files:** `StatsV1.cpp` (GPU initialization)
**Symptoms:** `GETGPUCount()` returns 0, GPU tile shows nothing.
**Fix:** Fallback to D3DKMTQueryStatistics or vendor APIs (nvapi, ADL).

### [ ] Network adapter names contain PDH instance junk
**Files:** `StatsV1.cpp` (FindNetworkAdapters)
**Fix:** Clean names or use IP Helper API (GetAdaptersAddresses) for friendly names.

### [ ] Per-disk enabled state not persisted in config
**Files:** `ConfigV1.h/.cpp`, `main.cpp`
**Fix:** Save/load disk/adapter enabled arrays.

### [ ] Disabled disks still counted in `GETDiskCount()`
**Files:** `StatsV1.h`
**Fix:** Add `GETEnabledDiskCount()`.

### [ ] F3 per-adapter toggles don't affect data collection
**Files:** `main.cpp` (updateStats), `StatsV1.h/.cpp`
**Fix:** Add per-adapter enabled checks in GETWiFi/GETEthernet methods.

### [ ] Resize grip hitbox (16x16) small on high-DPI
**Files:** `TileV1.h` (handleDrag)
**Fix:** Scale grip hitbox by screen DPI.

---

## Future Work

### [ ] GPU display name in tile bar labels
**Priority:** Low | **Files:** `StatsV1.h/.cpp`, `TileRenderer.h`
GPUInstance::displayName exists but not yet exposed. Add `GetGPUDisplayName(int)` and use in GPU tile.

### [ ] Temperature monitoring (CPU/GPU)
**Priority:** Medium | **Files:** `StatsV1.h/.cpp`
Via registry, MSR, nvapi/ADL, or WMI. Not all systems expose thermal data.

### [ ] System tray minimize
**Priority:** Medium | **Files:** `main.cpp`
Use `Shell_NotifyIconW`, hook WM_SIZE via raylib's GLFW window.

### [ ] CPU utilization history sparkline
**Priority:** Medium | **Files:** `TileRenderer.h` (renderCPUTile)
Store last N values in ring buffer, draw polyline below gauge.

### [ ] Configurable grid layout
**Priority:** Medium | **Files:** `LayoutConfig.h`, `ConfigV1.h`
Move grid config to config file, allow dynamic tile add/remove.

### [ ] Dark/Light mode auto-detection
**Priority:** Low | **Files:** `main.cpp`
Read `HKCU\...\AppsUseLightTheme` registry value at startup.

### [ ] BarV1 color gradient fill
**Priority:** Low | **Files:** `BarV1.cpp`
Use `DrawRectangleGradientH` with endColor in Theme.

### [ ] Window resize snapping / preset sizes
**Priority:** Low | **Files:** `main.cpp`
Snap window size to cell-multiples, or preset sizes (Compact/Normal/Large).

### [ ] Hover tooltips for bar labels
**Priority:** Low | **Files:** `TileRenderer.h`
On hover, draw tooltip with full label text.
