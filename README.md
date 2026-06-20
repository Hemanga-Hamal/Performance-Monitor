# Performance Monitor

Real-time system performance monitor for Windows. Displays CPU frequency/utilization, RAM, disk, and network metrics using raylib for rendering and the Windows PDH (Performance Data Helper) API for data collection.

## Features
- CPU: real-time frequency (MHz) and utilization (%)
- RAM: usage (GB) and utilization (%)
- Disk: C: drive usage (%)
- Network: Wi-Fi and Ethernet send/receive rates (Mbps)
- Dynamic network adapter detection (no hardcoded names)
- Threaded rendering at 30+ FPS with zero blocking calls
- Resizable window with auto-scaling UI elements

## Requirements
- Windows 7 or later
- CMake 3.20+ (or MinGW-g++ for legacy Makefile)

## Build

### CMake (recommended)
```powershell
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```
Output: `build\Release\perfmon.exe`

### Makefile (legacy)
Requires raylib installed at `C:/raylib/raylib`.
```powershell
make
```

## Usage
Run `perfmon.exe`. The UI is split into four quadrants:
- Top-left: CPU (gauge shows utilization, bar shows frequency)
- Top-right: RAM (gauge shows memory load)
- Bottom-left: Network (Wi-Fi and Ethernet send/receive rates)
- Bottom-right: Disk (C: drive utilization)

Close the window or press Escape to exit.

## License
See `resources/LICENSE`.
