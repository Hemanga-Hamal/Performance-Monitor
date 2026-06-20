#ifndef STATSV1_H
#define STATSV1_H

#ifndef _WIN32_WINNT
#define _WIN32_WINNT _WIN32_WINNT_WIN10
#endif
#include <windows.h>
#include <pdh.h>
#include <atomic>
#include <string>
#include <vector>

class StatsV1 {
public:
    struct DiskInfo {
        std::wstring name;
        float totalGB{0.0f};
        float usedGB{0.0f};
        float utilization{0.0f};
    };

private:
    // CPU measurements
    std::atomic<float> CPUUtilization{0.0f};
    std::atomic<float> CPUFrequency{0.0f};
    PDH_HQUERY cpuQuery{nullptr};
    PDH_HCOUNTER counterPerf{nullptr};
    PDH_HCOUNTER counterFreq{nullptr};
    LARGE_INTEGER freqCollectTime{};
    bool freqPrimed{false};
    FILETIME prevIdleTime{}, prevKernelTime{}, prevUserTime{};
    bool cpuUtilPrimed{false};
    std::string cpuModel;

    // RAM measurements
    MEMORYSTATUSEX memInfo{};
    float RAMTotal{0.0f};

    // Disk measurements
    std::vector<DiskInfo> disks;

    // GPU measurements
    PDH_HQUERY gpuQuery{nullptr};
    PDH_HCOUNTER gpuCounter{nullptr};
    std::wstring gpuName;
    std::string gpuModel;
    std::atomic<float> GPUUtilization{0.0f};
    LARGE_INTEGER gpuCollectTime{};
    bool gpuPrimed{false};

    // Network measurements
    struct NetworkCounters {
        PDH_HQUERY query{nullptr};
        PDH_HCOUNTER sendCounter{nullptr};
        PDH_HCOUNTER receiveCounter{nullptr};
        LARGE_INTEGER lastCollect{};
        bool primed{false};
        float sendRate{0.0f};
        float receiveRate{0.0f};
    };

    NetworkCounters wifi;
    NetworkCounters ethernet;

    // Discovered adapters for diagnostics
    std::vector<std::wstring> discoveredAdapters;

    LARGE_INTEGER lastCPUTime{};

    static bool InitializeNetworkCounter(NetworkCounters& counter, const wchar_t* adapterName);
    static void CleanupNetworkCounter(NetworkCounters& counter) noexcept;
    float GetNetworkRate(NetworkCounters& counter, PDH_HCOUNTER hCounter) noexcept;

public:
    StatsV1() noexcept;
    ~StatsV1() noexcept;

    StatsV1(const StatsV1&) = delete;
    StatsV1& operator=(const StatsV1&) = delete;
    StatsV1(StatsV1&&) noexcept = default;
    StatsV1& operator=(StatsV1&&) noexcept = default;

    [[nodiscard]] float GETCPUFrequency() noexcept;
    [[nodiscard]] float GETCPUtilization() noexcept;
    [[nodiscard]] const char* GETCPUModel() const noexcept { return cpuModel.c_str(); }

    [[nodiscard]] float GETRAMTotal() const noexcept { return RAMTotal; }
    [[nodiscard]] float GETRAMUsed() noexcept;
    [[nodiscard]] float GETRAMUtilization() noexcept;

    [[nodiscard]] int GETDiskCount() const noexcept { return static_cast<int>(disks.size()); }
    [[nodiscard]] const wchar_t* GETDiskName(int index) const noexcept;
    [[nodiscard]] float GETDiskTotal(int index) const noexcept;
    [[nodiscard]] float GETDiskUsed(int index) noexcept;
    [[nodiscard]] float GETDiskUtilization(int index) noexcept;

    [[nodiscard]] float GETGPUUtilization() noexcept;
    [[nodiscard]] const wchar_t* GETGPUName() const noexcept { return gpuName.c_str(); }
    [[nodiscard]] const char* GETGPUModel() const noexcept { return gpuModel.c_str(); }
    [[nodiscard]] bool IsGPUAvailable() const noexcept { return gpuQuery != nullptr; }

    [[nodiscard]] float GETWiFiSend() noexcept;
    [[nodiscard]] float GETWiFiReceive() noexcept;
    [[nodiscard]] float GETEthernetSend() noexcept;
    [[nodiscard]] float GETEthernetReceive() noexcept;

    [[nodiscard]] const std::vector<std::wstring>& GetDiscoveredAdapters() const noexcept { return discoveredAdapters; }
};

#endif
