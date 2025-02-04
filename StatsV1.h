#ifndef STATSV1_H
#define STATSV1_H

#include <windows.h>
#include <pdh.h>
#include <memory>
#include <string>

class StatsV1 {
private:
    // CPU measurements
    float CPUFrequency{0.0f};
    float CPUUtilization{0.0f};
    PDH_HQUERY cpuQuery{nullptr};
    PDH_HCOUNTER cpuFreqCounter{nullptr};
    LARGE_INTEGER lastCPUTime{};
    LARGE_INTEGER lastSysTime{};

    // RAM measurements
    MEMORYSTATUSEX memInfo{};
    float RAMTotal{0.0f};

    // Disk measurements
    static constexpr wchar_t DRIVE[] = L"C:\\";
    float DISKTotal{0.0f};

    // Network measurements
    struct NetworkCounters {
        PDH_HQUERY query{nullptr};
        PDH_HCOUNTER sendCounter{nullptr};
        PDH_HCOUNTER receiveCounter{nullptr};
        float sendRate{0.0f};
        float receiveRate{0.0f};
    };

    NetworkCounters wifi;
    NetworkCounters ethernet;

    // Helper functions
    static bool InitializeNetworkCounter(NetworkCounters& counter, const wchar_t* interfaceName);
    static void CleanupNetworkCounter(NetworkCounters& counter) noexcept;
    static float GetNetworkRate(PDH_HQUERY query, PDH_HCOUNTER counter) noexcept;

public:
    StatsV1() noexcept;
    ~StatsV1() noexcept;

    // Delete copy operations to prevent resource handling issues
    StatsV1(const StatsV1&) = delete;
    StatsV1& operator=(const StatsV1&) = delete;

    // Move operations
    StatsV1(StatsV1&&) noexcept = default;
    StatsV1& operator=(StatsV1&&) noexcept = default;

    // CPU metrics
    [[nodiscard]] float GETCPUFrequency() noexcept;
    [[nodiscard]] float GETCPUtilization() noexcept;

    // RAM metrics
    [[nodiscard]] float GETRAMTotal() const noexcept { return RAMTotal; }
    [[nodiscard]] float GETRAMUsed() noexcept;
    [[nodiscard]] float GETRAMUtilization() noexcept;

    // Disk metrics
    [[nodiscard]] float GETDISKTotal() const noexcept { return DISKTotal; }
    [[nodiscard]] float GETDISKUsed() noexcept;
    [[nodiscard]] float GETDISKUtilization() noexcept;

    // Network metrics
    [[nodiscard]] float GETWiFiSend() noexcept;
    [[nodiscard]] float GETWiFiReceive() noexcept;
    [[nodiscard]] float GETEthernetSend() noexcept;
    [[nodiscard]] float GETEthernetReceive() noexcept;
};

#endif // STATSV1_H