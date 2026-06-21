#ifndef DISKMONITOR_H
#define DISKMONITOR_H

#include <windows.h>
#include <string>
#include <vector>

class DiskMonitor {
public:
    struct DiskInfo {
        std::wstring name;
        float totalGB{0.0f};
        float usedGB{0.0f};
        float utilization{0.0f};
        bool enabled{true};
    };

    DiskMonitor() noexcept;
    ~DiskMonitor() noexcept = default;

    DiskMonitor(const DiskMonitor&) = delete;
    DiskMonitor& operator=(const DiskMonitor&) = delete;
    DiskMonitor(DiskMonitor&&) noexcept = default;
    DiskMonitor& operator=(DiskMonitor&&) noexcept = default;

    [[nodiscard]] int GetCount() const noexcept;
    [[nodiscard]] int GetEnabledCount() const noexcept;
    [[nodiscard]] const wchar_t* GetName(int index) const noexcept;
    [[nodiscard]] float GetTotalGB(int index) const noexcept;
    [[nodiscard]] float GetUsedGB(int index) noexcept;
    [[nodiscard]] float GetUtilization(int index) noexcept;
    [[nodiscard]] const std::vector<DiskInfo>& GetAll() const noexcept;
    void SetEnabled(int index, bool enabled) noexcept;

private:
    std::vector<DiskInfo> disks;
};

#endif
