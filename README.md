# Performance Monitor

Real-time system performance monitor for Windows. Displays CPU, RAM, GPU, disk, and network metrics using raylib for rendering and the Windows PDH (Performance Data Helper) API for data collection.

## Features
- **CPU:** real-time frequency (MHz) and utilization (%)
- **RAM:** usage (GB) and utilization (%)
- **GPU:** utilization per engine instance (multi-GPU support)
- **Disk:** all fixed drives with per-disk usage and utilization
- **Network:** Wi-Fi and Ethernet send/receive rates (Mbps)
- **Dynamic adapter/GPU discovery:** no hardcoded names
- **Tile UI:** drag, resize, snap-to-grid, collision avoidance
- **Themes:** Dark, Light, High Contrast (persisted across sessions)
- **Diagnostics overlay (F2):** live CPU/RAM/GPU/Disk/Network info, export to file
- **Settings overlay (F3):** per-tile, per-disk, per-adapter visibility toggles
- **CSV logging (F4):** toggle timestamped metrics logging to file
- **Config persistence:** theme, tile state, window position saved to `%APPDATA%\PerfMon\config.ini`
- **Threaded rendering** at 30+ FPS with zero blocking calls
- **Resizable window** with auto-scaling UI elements

## Requirements
- Windows 10 or later
- CMake 3.20+

## Build

### CMake (recommended)
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

## Usage
Run `perfmon.exe`. The UI shows a tile-based dashboard:
- **CPU:** gauge (utilization) + bar (frequency)
- **RAM:** gauge (memory load)
- **GPU:** one bar per GPU engine instance
- **Network:** Wi-Fi and Ethernet send/receive bars
- **Storage:** per-disk utilization bars

### Controls
| Key | Action |
|-----|--------|
| F2 | Toggle diagnostics overlay (scroll with mouse wheel) |
| F3 | Toggle settings (enable/disable tiles, disks, adapters) |
| F4 | Toggle CSV logging |
| Escape / Close | Exit (saves config) |

## License
See `resources/LICENSE`.
