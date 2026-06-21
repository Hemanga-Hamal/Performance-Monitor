#ifndef NETWORKMONITOR_H
#define NETWORKMONITOR_H

#ifndef _WIN32_WINNT
#define _WIN32_WINNT _WIN32_WINNT_WIN10
#endif
#include <windows.h>
#include <pdh.h>
#include <string>
#include <vector>
#include <atomic>

class NetworkMonitor {
public:
    struct AdapterInfo {
        std::wstring name;
        bool isWiFi{false};
        bool isEthernet{false};
        bool enabled{true};
    };

private:
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
    std::vector<std::wstring> discoveredAdapters;
    std::vector<AdapterInfo> adapterInfos;
    LARGE_INTEGER perfFrequency{};

    static bool InitializeNetworkCounter(NetworkCounters& nc, const wchar_t* adapterName);
    static void CleanupNetworkCounter(NetworkCounters& nc) noexcept;
    float GetNetworkRate(NetworkCounters& nc, PDH_HCOUNTER hCounter) noexcept;

public:
    NetworkMonitor();
    ~NetworkMonitor() noexcept;

    NetworkMonitor(const NetworkMonitor&) = delete;
    NetworkMonitor& operator=(const NetworkMonitor&) = delete;
    NetworkMonitor(NetworkMonitor&&) noexcept = default;
    NetworkMonitor& operator=(NetworkMonitor&&) noexcept = default;

    [[nodiscard]] float GetWiFiSend() noexcept;
    [[nodiscard]] float GetWiFiReceive() noexcept;
    [[nodiscard]] float GetEthernetSend() noexcept;
    [[nodiscard]] float GetEthernetReceive() noexcept;
    [[nodiscard]] const std::vector<std::wstring>& GetDiscoveredAdapters() const noexcept;
    [[nodiscard]] const std::vector<AdapterInfo>& GetAdapters() const noexcept;
    void SetAdapterEnabled(int index, bool enabled) noexcept;
};

#endif
