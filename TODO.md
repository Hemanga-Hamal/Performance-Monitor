# TODO — Performance Monitor

## Status: ALL MAIN FEATURES COMPLETE

Build: `cmake -B build && cmake --build build --config Release`
Tests: `build\Release\perfmon_tests.exe` — 63 passed, 0 failed
Lines: ~1800 across all source files

---

## Feature Areas (all done)

| # | Area | Status |
|---|---|---|
| 1 | GPU monitoring (PDH GPU Engine, utilization %) | Done |
| 2 | Network counter fix (swprintf_s bug, rate timing) | Done |
| 3 | Dynamic network adapter discovery (PDH enumeration) | Done |
| 4 | Multi-disk support (all DRIVE_FIXED enumerated) | Done |
| 5 | Tile-based UI (drag, resize, snap, collision avoidance) | Done |
| 6 | Diagnostics overlay (F2 — CPU/RAM/GPU/Disk/Network) | Done |
| 7 | Device settings (F3 — toggle which tiles display) | Done |
| 8 | Theming (Dark, Light, High Contrast presets) | Done |
| 9 | Landing page (theme picker, start button) | Done |
| 10 | Non-blocking stats collection (no Sleep()) | Done |
| 11 | CPU/GPU model name detection (registry + EnumDisplayDevices) | Done |
| 12 | Test suite (7 groups, 63 tests) | Done |

---

## Future Tasks

### [ ] Multi-GPU per-instance monitoring
**Why:** Currently only the first GPU Engine PDH instance is tracked. Multi-GPU workstations have multiple engines.
**Files:** `StatsV1.h`, `StatsV1.cpp`, `main.cpp`
**Approach:** Instead of `break` after first `PdhAddCounterW` success, add ALL non-_Total instances. Store in `vector<GPUInstance>`. Expose `GetGPUCount()`, `GetGPUName(i)`, `GetGPUUtilization(i)`. Add per-GPU tiles or tabbed GPU tile.
**Difficulty:** Medium

### [ ] Per-disk and per-adapter enable/disable
**Why:** F3 settings toggle whole categories (CPU/RAM/GPU/etc) but can't disable individual disks or network adapters.
**Files:** `main.cpp` (drawSettingsOverlay), `StatsV1.h`
**Approach:** Track per-disk and per-adapter `bool` in StatsV1. Expose via `SetDiskEnabled(i, bool)` / `SetAdapterEnabled(i, bool)`. Show per-item checkboxes in F3.
**Difficulty:** Medium

### [ ] BarV1 / GaugeV1 drawInRect method
**Why:** Widgets currently use global screen dimensions for sizing, then we override with setBaseSize/setDimensions per-tile. A `drawInRect(Rectangle)` method would be cleaner.
**Files:** `BarV1.h/.cpp`, `GaugeV1.h/.cpp`
**Approach:** Add `drawInRect(Rectangle bounds, const std::string& label)` that scales content to fit within bounds without needing setBaseSize/setDimensions calls.
**Difficulty:** Low

### [ ] Config file persistence
**Why:** Theme choice, enabled tiles, and window state lost on restart.
**Files:** New file `src/ConfigV1.h/.cpp`, `main.cpp`
**Approach:** Save/load a simple INI or JSON file in `%APPDATA%\PerfMon\config.json`. Store: theme index, tileEnabled[5], window position/size.
**Difficulty:** Low

### [ ] Temperature monitoring (CPU/GPU)
**Why:** No thermal data currently.
**Files:** `StatsV1.h/.cpp`
**Approach:** Use WMI `Win32_PerfFormattedData_Counters_ThermalZoneInformation` for CPU. For GPU: `nvapi` (NVIDIA only) or WMI. Fallback: not all systems expose thermal PDH counters.
**Difficulty:** Medium

### [ ] CSV logging
**Why:** No way to record stats over time for analysis.
**Files:** New file `src/Logger.h/.cpp`, `main.cpp`
**Approach:** Write timestamped CSV rows each stats update cycle. Column headers: CPU_Freq, CPU_Util, RAM_Used, GPU_Util, Disk_Util[], Net_Send/Recv. Use `std::ofstream` with buffered writes. Toggle with F4 or checkbox.
**Difficulty:** Low

### [ ] System tray minimize
**Why:** Window takes taskbar space when not in focus.
**Files:** `main.cpp`
**Approach:** Use `Shell_NotifyIconW` with `NIM_ADD` for tray icon. On minimize, hide window and show tray. On tray click, restore window. Needs `NOTIFYICONDATAW` and a custom `WM_TRAYICON` message handler via raylib's GLFW window hook or `SetWindowLongPtr`.
**Difficulty:** Medium

### [ ] Export diagnostics to text file
**Why:** F2 shows diagnostics but can't copy/save.
**Files:** `main.cpp`
**Approach:** Add "Save to file" button in diagnostics overlay or F2 long-press to save. Write same content as displayed to `diagnostics.txt` in working directory.
**Difficulty:** Low
