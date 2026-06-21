#include "RamMonitor.h"

namespace {
    constexpr float BYTES_TO_GB = 1024.0f * 1024.0f * 1024.0f;
}

RamMonitor::RamMonitor() noexcept {
    MEMORYSTATUSEX memInfo{};
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    if (GlobalMemoryStatusEx(&memInfo)) {
        totalGB = static_cast<float>(memInfo.ullTotalPhys) / BYTES_TO_GB;
    }
}

float RamMonitor::GetTotalGB() const noexcept {
    return totalGB;
}

float RamMonitor::GetUsedGB() noexcept {
    MEMORYSTATUSEX memInfo{};
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    if (GlobalMemoryStatusEx(&memInfo)) {
        return static_cast<float>(memInfo.ullTotalPhys - memInfo.ullAvailPhys) / BYTES_TO_GB;
    }
    return 0.0f;
}

float RamMonitor::GetUtilizationPercent() noexcept {
    MEMORYSTATUSEX memInfo{};
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    if (GlobalMemoryStatusEx(&memInfo)) {
        return static_cast<float>(memInfo.dwMemoryLoad);
    }
    return 0.0f;
}
