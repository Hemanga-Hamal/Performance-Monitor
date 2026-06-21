#ifndef GPUMONITOR_H
#define GPUMONITOR_H

#ifndef _WIN32_WINNT
#define _WIN32_WINNT _WIN32_WINNT_WIN10
#endif
#include <windows.h>
#include <pdh.h>
#include <string>
#include <vector>
#include <atomic>

class GpuMonitor {
public:
    struct GpuInfo {
        std::wstring engineName;
        std::string displayName;
        uint64_t vramTotalMB{0};
        float vramUsedPercent{0.0f};
        float utilization{0.0f};
        uint32_t clockMHz{0};
        PDH_HQUERY query{nullptr};
        PDH_HCOUNTER counter{nullptr};
        PDH_HQUERY memQuery{nullptr};
        PDH_HCOUNTER memCounter{nullptr};
        LARGE_INTEGER collectTime{};
        bool primed{false};
    };

private:
    std::vector<GpuInfo> gpus;
    bool comInitialized{false};

public:
    GpuMonitor();
    ~GpuMonitor() noexcept;

    GpuMonitor(const GpuMonitor&) = delete;
    GpuMonitor& operator=(const GpuMonitor&) = delete;
    GpuMonitor(GpuMonitor&&) noexcept = default;
    GpuMonitor& operator=(GpuMonitor&&) noexcept = default;

    [[nodiscard]] int GetCount() const noexcept;
    [[nodiscard]] float GetUtilization(int index = 0) noexcept;
    [[nodiscard]] float GetVramUsedPercent(int index = 0) noexcept;
    [[nodiscard]] const char* GetDisplayName(int index = 0) const noexcept;
    [[nodiscard]] const wchar_t* GetEngineName(int index = 0) const noexcept;
    [[nodiscard]] uint64_t GetVramTotalMB(int index = 0) const noexcept;
    [[nodiscard]] uint32_t GetClockMHz(int index = 0) const noexcept;
    [[nodiscard]] bool IsAvailable() const noexcept;
    [[nodiscard]] const std::vector<GpuInfo>& GetAll() const noexcept;
};

#endif
