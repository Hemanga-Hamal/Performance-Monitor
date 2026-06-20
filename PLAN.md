# Project Plan: Performance Monitor

## Status: Production-Ready — All Phases Complete

## Overview
Windows-only C++ performance monitor (~1800 lines) using raylib 5.5 and the PDH API. Monitors CPU frequency/utilization, RAM, GPU utilization, disk usage, and network throughput in real time. Features a tile-based dashboard UI with drag-to-arrange tiles, theme support, device diagnostics, and a 63-test suite.

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
  main.cpp              # Entry point, landing page, tile dashboard, F2 diagnostics, F3 settings
  StatsV1.cpp / .h      # PDH stats collector: CPU freq/util, RAM used/util, GPU util,
                        # multi-disk enumeration, network adapter discovery, rate collection
  BarV1.cpp / .h        # Horizontal progress bar (theme, dimensions, config, auto-scale)
  GaugeV1.cpp / .h      # Circular arc gauge (clamp, DegToRad, DrawArc, auto-scale)
  TileV1.h              # Grid tile widget (computeBounds, handleDrag, snapToGrid, drawFrame,
                        # snapPreview, collision avoidance, resize grip)
  ThemeV1.h             # Color theme: Dark, Light, HighContrast static presets
  tests/
    test_main.cpp        # Test runner (calls all groups)
    test_harness.h       # Shared macros: TEST, CHECK, CHECK_EQ, CHECK_RANGE
    test_cpu.cpp         # CPUTests: frequency, utilization, model name
    test_ram.cpp         # RAMTests: total, used, utilization
    test_gpu.cpp         # GPUTests: available, utilization, model, PDH name
    test_disk.cpp        # DiskTests: count, per-disk name/total/used/util
    test_network.cpp     # NetworkTests: WiFi/Ethernet send/receive, adapter list
    test_graphics.cpp    # GraphicsTests: BarV1 theme/dims/config/sizing, GaugeV1 clamp/presets
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
- Dynamic network adapter discovery via PdhEnumObjectItemsW with keyword matching
- Network swprintf_s %%s bug fixed (adapter name was replaced by direction string)
- Thread safety: atomic cache values, single-writer pattern

### Phase 3: Build & Distribution — DONE
- CMakeLists.txt targets: perfmon.exe + perfmon_tests.exe
- MSVC #pragma removed, linking in CMake target_link_libraries
- Unused includes removed
- GaugeV1 missing setters implemented (setTotalAngle, setStartAngle, etc.)
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
- Collision avoidance: pushed tiles shift down/wrap on overlap
- 2×4 grid: CPU(0,0), RAM(1,0), GPU(0,1), Network(1,1), Storage(0,2 span=2)
- Content auto-scales to tile size (setBaseSize, setDimensions maxSize)

### Phase 6: Overlays — DONE
- F2: Scrollable diagnostics (CPU/RAM/GPU/Disk/Network with live values)
- F3: Device visibility toggles with device-specific labels
- Landing page: theme selector (Dark/Light/HighContrast), start button

### Phase 7: Testing — DONE
- 63 tests, 7 groups, 0 failures
- Cover: CPU freq/util/model, RAM total/used/util, GPU avail/util/model, disk enumeration, network adapters, BarV1/GaugeV1 widget logic

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

### Network Adapter Discovery
1. `PdhEnumObjectItemsW(L"Network Interface")` to get all adapter names
2. Keyword matching: Wi-Fi keywords (wi-fi, wifi, wireless, wlan, 802.11)
3. Exclusion filters: bluetooth, virtual, loopback, teredo, isatap
4. Fallback: single adapter → assign to ethernet

## Known Limitations
- Single-window raylib (no multi-monitor or separate diagnostic window)
- GPU: only first non-_Total PDH engine instance (not per-engine on multi-GPU)
- Network: only one Wi-Fi and one Ethernet adapter (not all adapters)
- No config persistence (theme/tile state lost on restart)
- No temperature, fan speed, or power monitoring
