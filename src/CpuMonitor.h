#ifndef CPUMONITOR_H
#define CPUMONITOR_H

#ifndef _WIN32_WINNT
#define _WIN32_WINNT _WIN32_WINNT_WIN10
#endif
#include <windows.h>
#include <pdh.h>
#include <string>
#include <atomic>

class CpuMonitor {
public:
    CpuMonitor() noexcept;
    ~CpuMonitor() noexcept;

    CpuMonitor(const CpuMonitor&) = delete;
    CpuMonitor& operator=(const CpuMonitor&) = delete;
    CpuMonitor(CpuMonitor&&) noexcept = default;
    CpuMonitor& operator=(CpuMonitor&&) noexcept = default;

    [[nodiscard]] float GetFrequency() noexcept;
    [[nodiscard]] float GetUtilization() noexcept;
    [[nodiscard]] const char* GetModel() const noexcept;

private:
    std::atomic<float> utilization{0.0f};
    std::atomic<float> frequency{0.0f};
    std::string model;
    PDH_HQUERY query{nullptr};
    PDH_HCOUNTER counterPerf{nullptr};
    PDH_HCOUNTER counterFreq{nullptr};
    LARGE_INTEGER freqCollectTime{};
    bool freqPrimed{false};
    FILETIME prevIdleTime{}, prevKernelTime{}, prevUserTime{};
    bool utilPrimed{false};
};

#endif
