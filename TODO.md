# TODO — Performance Monitor

## Status

Build: `cmake -B build && cmake --build build --config Release`
Tests: `build\Release\perfmon_tests.exe` — 63 passed, 0 failed

---

## Completed Features

| # | Area | Notes |
|---|------|-------|
| 1 | CPU monitoring | Frequency (PDH), utilization (GetSystemTimes), model (registry) |
| 2 | RAM monitoring | Total/used/util (GlobalMemoryStatusEx) |
| 3 | GPU monitoring | Multi-GPU per-instance PDH queries via `PdhExpandWildCardPathW` |
| 4 | Disk monitoring | All DRIVE_FIXED enumerated via GetLogicalDrives |
| 5 | Network monitoring | WiFi/Ethernet send/receive via PDH; adapter discovery via `PdhExpandWildCardPathW` |
| 6 | Tile UI | 4x6 grid, drag title bar to move, drag grip to resize, snap-to-grid, ghost preview |
| 7 | Collision avoidance | `resolveCollisions()` tries all grid cells (0..gridRows × 0..gridCols) for displaced tiles |
| 8 | Tile auto-span | SpanCols/SpanRows recomputed from bounds.width/cellW ratio on snap |
| 9 | Diagnostics overlay (F2) | Scrollable panel with scissor clipping, Save-to-file button |
| 10 | Settings overlay (F3) | Per-tile, per-disk, per-adapter toggle with live info |
| 11 | CSV logging (F4) | Toggle on/off, writes timestamped rows to working directory |
| 12 | Theming | Dark, Light, High Contrast with shadow/accent/panel colors |
| 13 | Landing page | 3 theme cards with color swatches, hover/selected states, "Start Monitoring" button |
| 14 | Config persistence | Theme, tileEnabled[], window pos/size saved to `%APPDATA%\PerfMon\config.ini` |
| 15 | Status bar | 30px bar at window bottom with key hints and log status |
| 16 | Multi-GPU labels | "GPU 1", "GPU 2" for multi-GPU; "Utilization %" for single GPU |
| 17 | Non-blocking stats | All PDH queries use QPC-timed two-phase pattern, zero Sleep() calls |
| 18 | Test suite | 63 tests across 7 groups, all pass |
| 19 | Bug fixes | GetNetworkRate early-return, swprintf_s buffer size, DiskCount.load() on non-atomic |

---

## Known Issues

### [ ] CPU frequency reports 0 on some systems
**Why:** `\Processor Information(_Total)\Processor Frequency` counter may not exist (especially on older or non-Intel CPUs).
**Files:** `StatsV1.cpp` (GETCPUFrequency)
**Symptoms:** CPU frequency bar shows 0.0. CPU utilization still works.
**Fix:** Fallback to reading base frequency from registry (`HKLM\HARDWARE\DESCRIPTION\System\CentralProcessor\0\~MHz`) or use `\Processor(_Total)\% Processor Performance` with the nominal frequency.

### [ ] CPU model text truncated for long names (e.g. "AMD Ryzen 9 5950X 16-Core Processor")
**Why:** Font auto-shrinks but has a minimum bound of ~7px. At small tile sizes text becomes unreadable.
**Files:** `main.cpp` (CPU tile section)
**Fix:** Multi-line text wrapping or scroll text like a marquee on small tiles.

### [ ] GPU PDH counter may not exist on all systems
**Why:** `\GPU Engine(*)\Utilization Percentage` requires GPU drivers to expose the counter. Some laptops with hybrid graphics or basic display adapters don't have it.
**Files:** `StatsV1.cpp` (GPU initialization)
**Symptoms:** `GETGPUCount()` returns 0, `IsGPUAvailable()` returns false, GPU tile shows nothing.
**Fix:** Fallback to reading GPU usage via `D3DKMTQueryStatistics` or `nvapi` / `ADL` for NVIDIA/AMD.

### [ ] Network adapter names from PDH are long and contain garbage on some NICs
**Why:** `PdhExpandWildCardPathW` returns the full PDH instance name which can be "Intel[R] Ethernet Connection I219-V #2" or similar.
**Files:** `StatsV1.cpp` (FindNetworkAdapters)
**Fix:** Truncate or clean adapter names. Use IP Helper API (`GetAdaptersAddresses`) for friendly names.

### [ ] Per-disk enabled state not persisted in config
**Why:** `ConfigV1` saves/loads tileEnabled[5] but not per-disk `DiskInfo::enabled`.
**Files:** `ConfigV1.h/.cpp`, `main.cpp`
**Fix:** Save/load an array of disk enabled bools. Same for per-adapter.

### [ ] Disabled disks still counted in `GETDiskCount()`
**Why:** `GETDiskCount()` returns `disks.size()` regardless of per-disk enabled state.
**Files:** `StatsV1.h`
**Symptoms:** F3 "Storage Tile" shows count of all disks including disabled ones.
**Fix:** Add `GETEnabledDiskCount()` method that counts only enabled disks.

### [ ] F3 per-adapter toggles don't affect which adapters are monitored
**Why:** `SetAdapterEnabled()` sets a bool but the update thread still collects data for WiFi/Ethernet regardless.
**Files:** `main.cpp` (updateStats), `StatsV1.h/.cpp`
**Fix:** Add per-adapter enabled checks in `GETWiFiSend/Receive` and `GETEthernetSend/Receive`.

### [ ] Keyboard shortcuts (F2/F3/F4) only work when raylib window is focused
**Why:** This is inherent to raylib/GLFW input handling. No fix needed but worth documenting.

### [ ] `BarV1::draw()` text can still clip above tile bounds when tile is very small
**Why:** `textY` is clamped to min 4px but content may still overflow the tile edge even with the scissor.
**Files:** `BarV1.cpp` line 73-74
**Fix:** Pass a minimum Y boundary to the draw method, or clip in main.cpp with `BeginScissorMode`.

### [ ] Text size for overlays (F2/F3) still tied to window `fontSize` not overlay size
**Why:** Overlay fonts derived from `fontSize = max(18 * hScale, 10)` which scales with window, not overlay panel.
**Files:** `main.cpp` (drawDiagnosticsOverlay, drawSettingsOverlay)
**Fix:** Compute overlay fonts from panel dimensions instead of window dimensions.

### [ ] Snap preview on release can visually "jump" — no smooth interpolation
**Why:** Tiles snap instantly to grid on release. No animation/tween.
**Files:** `TileV1.h` (snapToGrid)
**Fix:** Animate the bounds from current position to target over a few frames.

### [ ] Resize grip hitbox (16x16) is small on high-DPI screens
**Why:** Razer-edge click target. On 4K screens, 16px is tiny.
**Files:** `TileV1.h` (handleDrag line 115)
**Fix:** Scale the grip hitbox based on screen dimensions or DPI.

---

## Future Work

### [ ] Real GPU model name mapping for multi-GPU labels
**Why:** Currently shows "GPU 1", "GPU 2" labels. `EnumDisplayDevicesW` can enumerate all GPUs but timing/matching to PDH instances is non-trivial.
**Files:** `StatsV1.cpp` (GPU init), `main.cpp` (GPU tile)
**Approach:** The `GPUInstance::displayName` field was added but not yet exposed via public API. Add `GetGPUDisplayName(int)` to `StatsV1` and use it in the GPU tile for labels.
**Difficulty:** Low (infrastructure exists, needs wiring)

### [ ] System tray minimize
**Why:** Window stays in taskbar when minimized.
**Files:** `main.cpp`
**Approach:** Use `Shell_NotifyIconW` with `NIM_ADD`. Hook into raylib's GLFW window for `WM_SIZE` to detect minimize. Tray icon click restores window. Needs `NOTIFYICONDATAW` struct and message loop integration.
**Difficulty:** Medium (needs GLFW message hook)

### [ ] Temperature monitoring (CPU/GPU)
**Why:** No thermal data displayed.
**Files:** `StatsV1.h/.cpp`
**Approach:** Read CPU temp from registry `HKLM\HARDWARE\DESCRIPTION\System\CentralProcessor\0` or MSR. GPU temp via `nvapi` (NVIDIA), `ADL` (AMD), or `WMI Win32_PerfFormattedData_Counters_ThermalZoneInformation`. Not all systems expose thermal data.
**Difficulty:** Medium-High (vendor-specific APIs)

### [ ] Window resize snapping / preset sizes
**Why:** Free-form resize can create awkward tile proportions.
**Files:** `main.cpp`
**Approach:** Snap window size to cell-multiples (e.g. multiples of 300x133 for 4x6 grid at 1200x800). Or provide preset sizes like "Compact", "Normal", "Large".
**Difficulty:** Low

### [ ] GPU per-instance display name in tile bar labels
**Why:** `GETGPUName()` returns PDH instance name like "gpu_0_eng_3". `GETGPUDisplayName()` not yet exposed.
**Files:** `StatsV1.h/.cpp`, `main.cpp`
**Approach:** Add `const char* GETGPUDisplayName(int index)` method, use it in GPU tile label. Requires converting displayName (std::string) to const char* return — use cached C string or wide char conversion.
**Difficulty:** Low

### [ ] Per-tile font size independent tuning
**Why:** All tiles use same `tileTitleSize` formula. Some tiles need smaller title font.
**Files:** `main.cpp` (renderLoop)
**Approach:** Let each tile define a font scale factor in TileConfig. Apply per-tile when calling `drawFrame`.
**Difficulty:** Low

### [ ] BarV1 color gradient fill
**Why:** Bars currently use solid color fill. Gradient from dim to bright would look more polished.
**Files:** `BarV1.cpp`
**Approach:** Use `DrawRectangleGradientH` (raylib) for the fill bar. Requires defining a second color (endColor) in BarV1::Theme.
**Difficulty:** Low

### [ ] CPU utilization history sparkline
**Why:** Only shows current value. A mini line chart would be more informative.
**Files:** `main.cpp` (CPU tile), new file or inline
**Approach:** Store last N (e.g. 60) utilization values in a ring buffer. Draw a polyline in the tile content area below the gauge.
**Difficulty:** Medium

### [ ] Configurable grid layout (not hardcoded 4x6)
**Why:** Grid cols/rows and tile positions are hardcoded.
**Files:** `main.cpp`
**Approach:** Store gridCols, gridRows, and initial tile positions in ConfigV1. Allow user to add/remove tiles dynamically.
**Difficulty:** Medium

### [ ] Hover tooltips for GPU/Disk bar labels
**Why:** Truncated bar labels don't show full text.
**Files:** `main.cpp` (renderLoop)
**Approach:** On mouse hover over a bar, draw a tooltip rectangle with the full label text. Use `CheckCollisionPointRec` against the bar rectangle.
**Difficulty:** Low

### [ ] Dark/Light mode auto-detection
**Why:** User must manually select theme.
**Files:** `main.cpp`
**Approach:** Read `HKCU\Software\Microsoft\Windows\CurrentVersion\Themes\Personalize\AppsUseLightTheme` registry value at startup.
**Difficulty:** Low
