#ifndef RAMMONITOR_H
#define RAMMONITOR_H

#ifndef _WIN32_WINNT
#define _WIN32_WINNT _WIN32_WINNT_WIN10
#endif
#include <windows.h>
#include <atomic>

class RamMonitor {
public:
    RamMonitor() noexcept;
    ~RamMonitor() noexcept = default;

    RamMonitor(const RamMonitor&) = delete;
    RamMonitor& operator=(const RamMonitor&) = delete;
    RamMonitor(RamMonitor&&) noexcept = default;
    RamMonitor& operator=(RamMonitor&&) noexcept = default;

    [[nodiscard]] float GetTotalGB() const noexcept;
    [[nodiscard]] float GetUsedGB() noexcept;
    [[nodiscard]] float GetUtilizationPercent() noexcept;

private:
    float totalGB{0.0f};
};

#endif
