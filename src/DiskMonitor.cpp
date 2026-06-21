#include "DiskMonitor.h"

namespace {
    constexpr float BYTES_TO_GB = 1024.0f * 1024.0f * 1024.0f;
}

DiskMonitor::DiskMonitor() noexcept {
    DWORD drives = GetLogicalDrives();
    for (int i = 0; i < 26; i++) {
        if (!(drives & (1 << i))) continue;
        wchar_t root[4] = {static_cast<wchar_t>(L'A' + i), L':', L'\\', L'\0'};
        if (GetDriveTypeW(root) != DRIVE_FIXED) continue;

        ULARGE_INTEGER totalBytes, freeBytes;
        if (!GetDiskFreeSpaceExW(root, nullptr, &totalBytes, &freeBytes)) continue;

        DiskInfo disk;
        wchar_t volName[MAX_PATH + 1] = {};
        if (GetVolumeInformationW(root, volName, MAX_PATH, nullptr, nullptr, nullptr, nullptr, 0) && volName[0] != L'\0') {
            disk.name = volName;
        } else {
            disk.name = root;
        }
        disk.totalGB = static_cast<float>(totalBytes.QuadPart) / BYTES_TO_GB;
        disk.usedGB = static_cast<float>(totalBytes.QuadPart - freeBytes.QuadPart) / BYTES_TO_GB;
        disk.utilization = disk.totalGB > 0.0f ? (disk.usedGB / disk.totalGB) * 100.0f : 0.0f;
        disks.push_back(disk);
    }
}

int DiskMonitor::GetCount() const noexcept {
    return static_cast<int>(disks.size());
}

int DiskMonitor::GetEnabledCount() const noexcept {
    int count = 0;
    for (const auto& d : disks) {
        if (d.enabled) count++;
    }
    return count;
}

const wchar_t* DiskMonitor::GetName(int index) const noexcept {
    if (index < 0 || index >= static_cast<int>(disks.size())) return L"";
    return disks[index].name.c_str();
}

float DiskMonitor::GetTotalGB(int index) const noexcept {
    if (index < 0 || index >= static_cast<int>(disks.size())) return 0.0f;
    return disks[index].totalGB;
}

float DiskMonitor::GetUsedGB(int index) noexcept {
    if (index < 0 || index >= static_cast<int>(disks.size())) return 0.0f;
    const wchar_t* root = disks[index].name.c_str();
    ULARGE_INTEGER totalBytes, freeBytes;
    if (!GetDiskFreeSpaceExW(root, nullptr, &totalBytes, &freeBytes)) return 0.0f;
    disks[index].totalGB = static_cast<float>(totalBytes.QuadPart) / BYTES_TO_GB;
    float used = static_cast<float>(totalBytes.QuadPart - freeBytes.QuadPart) / BYTES_TO_GB;
    disks[index].usedGB = used;
    return used;
}

float DiskMonitor::GetUtilization(int index) noexcept {
    float used = GetUsedGB(index);
    float total = GetTotalGB(index);
    return total > 0.0f ? (used / total) * 100.0f : 0.0f;
}

const std::vector<DiskMonitor::DiskInfo>& DiskMonitor::GetAll() const noexcept {
    return disks;
}

void DiskMonitor::SetEnabled(int index, bool enabled) noexcept {
    if (index >= 0 && index < static_cast<int>(disks.size())) {
        disks[index].enabled = enabled;
    }
}
