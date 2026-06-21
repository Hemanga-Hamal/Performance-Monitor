#ifndef STATSCOLLECTOR_H
#define STATSCOLLECTOR_H

#include "CpuMonitor.h"
#include "GpuMonitor.h"
#include "RamMonitor.h"
#include "DiskMonitor.h"
#include "NetworkMonitor.h"
#include <string>
#include <vector>

class StatsCollector {
private:
    CpuMonitor cpu;
    GpuMonitor gpu;
    RamMonitor ram;
    DiskMonitor disk;
    NetworkMonitor network;

public:
    StatsCollector() noexcept = default;
    ~StatsCollector() noexcept = default;

    StatsCollector(const StatsCollector&) = delete;
    StatsCollector& operator=(const StatsCollector&) = delete;
    StatsCollector(StatsCollector&&) noexcept = default;
    StatsCollector& operator=(StatsCollector&&) noexcept = default;

    [[nodiscard]] float GETCPUFrequency() noexcept { return cpu.GetFrequency(); }
    [[nodiscard]] float GETCPUtilization() noexcept { return cpu.GetUtilization(); }
    [[nodiscard]] const char* GETCPUModel() const noexcept { return cpu.GetModel(); }

    [[nodiscard]] float GETRAMTotal() const noexcept { return ram.GetTotalGB(); }
    [[nodiscard]] float GETRAMUsed() noexcept { return ram.GetUsedGB(); }
    [[nodiscard]] float GETRAMUtilization() noexcept { return ram.GetUtilizationPercent(); }

    [[nodiscard]] int GETDiskCount() const noexcept { return disk.GetCount(); }
    [[nodiscard]] int GETEnabledDiskCount() const noexcept { return disk.GetEnabledCount(); }
    [[nodiscard]] const wchar_t* GETDiskName(int index) const noexcept { return disk.GetName(index); }
    [[nodiscard]] float GETDiskTotal(int index) const noexcept { return disk.GetTotalGB(index); }
    [[nodiscard]] float GETDiskUsed(int index) noexcept { return disk.GetUsedGB(index); }
    [[nodiscard]] float GETDiskUtilization(int index) noexcept { return disk.GetUtilization(index); }

    [[nodiscard]] int GETGPUCount() const noexcept { return gpu.GetCount(); }
    [[nodiscard]] float GETGPUUtilization(int index = 0) noexcept { return gpu.GetUtilization(index); }
    [[nodiscard]] const wchar_t* GETGPUName(int index = 0) const noexcept { return gpu.GetEngineName(index); }
    [[nodiscard]] const char* GETGPUDisplayName(int index = 0) const noexcept { return gpu.GetDisplayName(index); }
    [[nodiscard]] const char* GETGPUModel() const noexcept { return gpu.GetDisplayName(0); }
    [[nodiscard]] bool IsGPUAvailable() const noexcept { return gpu.IsAvailable(); }

    [[nodiscard]] float GETWiFiSend() noexcept { return network.GetWiFiSend(); }
    [[nodiscard]] float GETWiFiReceive() noexcept { return network.GetWiFiReceive(); }
    [[nodiscard]] float GETEthernetSend() noexcept { return network.GetEthernetSend(); }
    [[nodiscard]] float GETEthernetReceive() noexcept { return network.GetEthernetReceive(); }

    [[nodiscard]] const std::vector<std::wstring>& GetDiscoveredAdapters() const noexcept { return network.GetDiscoveredAdapters(); }
    [[nodiscard]] const std::vector<NetworkMonitor::AdapterInfo>& GetAdapters() const noexcept { return network.GetAdapters(); }
    [[nodiscard]] const std::vector<DiskMonitor::DiskInfo>& GetDisks() const noexcept { return disk.GetAll(); }
    void SetDiskEnabled(int index, bool enabled) noexcept { disk.SetEnabled(index, enabled); }
    void SetAdapterEnabled(int index, bool enabled) noexcept { network.SetAdapterEnabled(index, enabled); }
};

#endif
