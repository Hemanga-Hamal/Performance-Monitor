#ifndef DATATYPES_H
#define DATATYPES_H

#ifndef _WIN32_WINNT
#define _WIN32_WINNT _WIN32_WINNT_WIN10
#endif
#include <windows.h>
#include <pdh.h>
#include <string>
#include <vector>

struct DiskInfo {
    std::wstring name;
    float totalGB{0.0f};
    float usedGB{0.0f};
    float utilization{0.0f};
    bool enabled{true};
};

struct AdapterInfo {
    std::wstring name;
    bool isWiFi{false};
    bool isEthernet{false};
    bool enabled{true};
};

struct GPUInstance {
    PDH_HQUERY query{nullptr};
    PDH_HCOUNTER counter{nullptr};
    PDH_HQUERY vramQuery{nullptr};
    PDH_HCOUNTER vramCounter{nullptr};
    std::wstring name;
    std::string displayName;
    LARGE_INTEGER collectTime{};
    bool primed{false};
    float cachedUtilization{0.0f};
    float cachedVRAMUsed{0.0f};
    float vramTotalGB{0.0f};
    int clockSpeedMHz{0};
};

#endif
